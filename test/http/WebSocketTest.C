/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WConfig.h"

#include "Wt/AsioWrapper/asio.hpp"
#include "Wt/AsioWrapper/system_error.hpp"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WServer.h"

#include "web/FileUtils.h"

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

namespace asio = Wt::AsioWrapper::asio;
typedef asio::ip::tcp tcp;

const std::chrono::seconds OperationTimeout(5);
const std::size_t MaxHttpResponseSize = 16 * 1024 * 1024;
const std::size_t MaxHttpHeaderSize = 64 * 1024;
const std::uint64_t MaxFramePayloadSize = 1024 * 1024;

std::string lowerCase(const std::string& value)
{
  std::string result = value;
  for (std::size_t i = 0; i < result.size(); ++i) {
    result[i] = static_cast<char>(
      std::tolower(static_cast<unsigned char>(result[i])));
  }
  return result;
}

std::string trim(const std::string& value)
{
  std::size_t begin = 0;
  while (begin < value.size() &&
         std::isspace(static_cast<unsigned char>(value[begin]))) {
    ++begin;
  }

  std::size_t end = value.size();
  while (end > begin &&
         std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }

  return value.substr(begin, end - begin);
}

bool containsToken(const std::string& value, const std::string& token)
{
  const std::string expected = lowerCase(token);
  std::size_t begin = 0;

  while (begin <= value.size()) {
    const std::size_t comma = value.find(',', begin);
    const std::size_t end = comma == std::string::npos
      ? value.size() : comma;
    if (lowerCase(trim(value.substr(begin, end - begin))) == expected) {
      return true;
    }
    if (comma == std::string::npos) {
      break;
    }
    begin = comma + 1;
  }

  return false;
}

struct HttpResponse
{
  int status;
  std::map<std::string, std::string> headers;
  std::string body;

  bool hasHeader(const std::string& name) const
  {
    return headers.find(lowerCase(name)) != headers.end();
  }

  std::string header(const std::string& name) const
  {
    std::map<std::string, std::string>::const_iterator i
      = headers.find(lowerCase(name));
    if (i == headers.end()) {
      return std::string();
    }
    return i->second;
  }
};

HttpResponse parseHttpResponse(const std::string& rawResponse)
{
  const std::size_t headerEnd = rawResponse.find("\r\n\r\n");
  if (headerEnd == std::string::npos) {
    throw std::runtime_error("HTTP response has no complete header");
  }

  HttpResponse response;
  response.status = 0;

  std::istringstream headers(rawResponse.substr(0, headerEnd));
  std::string line;
  if (!std::getline(headers, line)) {
    throw std::runtime_error("HTTP response has no status line");
  }
  if (!line.empty() && line[line.size() - 1] == '\r') {
    line.erase(line.size() - 1);
  }

  std::istringstream statusLine(line);
  std::string version;
  if (!(statusLine >> version >> response.status) ||
      version.compare(0, 5, "HTTP/") != 0) {
    throw std::runtime_error("HTTP response has an invalid status line");
  }

  while (std::getline(headers, line)) {
    if (!line.empty() && line[line.size() - 1] == '\r') {
      line.erase(line.size() - 1);
    }
    const std::size_t colon = line.find(':');
    if (colon == std::string::npos) {
      throw std::runtime_error("HTTP response has an invalid header");
    }

    const std::string name = lowerCase(trim(line.substr(0, colon)));
    const std::string value = trim(line.substr(colon + 1));
    std::map<std::string, std::string>::iterator existing
      = response.headers.find(name);
    if (existing == response.headers.end()) {
      response.headers[name] = value;
    } else {
      existing->second += "," + value;
    }
  }

  response.body = rawResponse.substr(headerEnd + 4);
  return response;
}

class TimedConnection
{
public:
  explicit TimedConnection(unsigned short port)
    : socket_(ioService_),
      timer_(ioService_)
  {
    connect(port);
  }

  ~TimedConnection()
  {
    Wt::AsioWrapper::error_code ignored;
    socket_.shutdown(tcp::socket::shutdown_both, ignored);
    socket_.close(ignored);
  }

  void write(const std::string& data)
  {
    const IoResult result = runIo(
      [this, &data](const IoHandler& handler) {
        asio::async_write(socket_, asio::buffer(data), handler);
      },
      "write");
    requireSuccess(result, data.size(), "write");
  }

  void write(const std::vector<unsigned char>& data)
  {
    const IoResult result = runIo(
      [this, &data](const IoHandler& handler) {
        asio::async_write(socket_, asio::buffer(data), handler);
      },
      "write");
    requireSuccess(result, data.size(), "write");
  }

  std::string readUntilClose()
  {
    std::string result;

    for (;;) {
      std::array<char, 8192> buffer;
      const IoResult readResult = runIo(
        [this, &buffer](const IoHandler& handler) {
          socket_.async_read_some(asio::buffer(buffer), handler);
        },
        "read");

      result.append(buffer.data(), readResult.size);
      if (result.size() > MaxHttpResponseSize) {
        throw std::runtime_error("HTTP response is unreasonably large");
      }
      if (readResult.error == asio::error::eof) {
        break;
      }
      if (readResult.error) {
        throw Wt::AsioWrapper::system_error(readResult.error);
      }
      if (readResult.size == 0) {
        throw std::runtime_error("HTTP read completed without data");
      }
    }

    return result;
  }

  HttpResponse readResponseHeaders()
  {
    asio::streambuf buffer(MaxHttpHeaderSize);
    const IoResult result = runIo(
      [this, &buffer](const IoHandler& handler) {
        asio::async_read_until(socket_, buffer, "\r\n\r\n", handler);
      },
      "read headers");
    if (result.error) {
      throw Wt::AsioWrapper::system_error(result.error);
    }

    std::istream input(&buffer);
    const std::string raw((std::istreambuf_iterator<char>(input)),
                          std::istreambuf_iterator<char>());
    const std::size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
      throw std::runtime_error("HTTP response has no complete header");
    }

    pending_.insert(pending_.end(), raw.begin() + headerEnd + 4, raw.end());
    return parseHttpResponse(raw.substr(0, headerEnd + 4));
  }

  std::vector<unsigned char> read(std::size_t size)
  {
    std::vector<unsigned char> result;
    result.reserve(size);

    const std::size_t buffered = std::min(size, pending_.size());
    result.insert(result.end(), pending_.begin(), pending_.begin() + buffered);
    pending_.erase(pending_.begin(), pending_.begin() + buffered);

    if (result.size() < size) {
      const std::size_t offset = result.size();
      result.resize(size);
      const IoResult readResult = runIo(
        [this, &result, offset, size](const IoHandler& handler) {
          asio::async_read(socket_,
                           asio::buffer(result.data() + offset,
                                        size - offset),
                           handler);
        },
        "read");
      requireSuccess(readResult, size - offset, "read");
    }

    return result;
  }

private:
  typedef std::function<void(const Wt::AsioWrapper::error_code&,
                             std::size_t)> IoHandler;

  struct IoResult
  {
    Wt::AsioWrapper::error_code error;
    std::size_t size;
  };

  asio::io_service ioService_;
  tcp::socket socket_;
  asio::steady_timer timer_;
  std::vector<unsigned char> pending_;

  void connect(unsigned short port)
  {
    ioService_.restart();

    bool completed = false;
    bool timedOut = false;
    Wt::AsioWrapper::error_code operationError;

    timer_.expires_after(OperationTimeout);
    timer_.async_wait(
      [this, &completed, &timedOut](
        const Wt::AsioWrapper::error_code& error) {
        if (!error && !completed) {
          timedOut = true;
          Wt::AsioWrapper::error_code ignored;
          socket_.close(ignored);
        }
      });

    const tcp::endpoint endpoint(asio::ip::address_v4::loopback(), port);
    socket_.async_connect(
      endpoint,
      [this, &completed, &operationError](
        const Wt::AsioWrapper::error_code& error) {
        operationError = error;
        completed = true;
        timer_.cancel();
      });

    ioService_.run();

    if (timedOut) {
      throw std::runtime_error("connect timed out");
    }
    if (!completed) {
      throw std::runtime_error("connect did not complete");
    }
    if (operationError) {
      throw Wt::AsioWrapper::system_error(operationError);
    }
  }

  template <typename Start>
  IoResult runIo(const Start& start, const std::string& operation)
  {
    ioService_.restart();

    bool completed = false;
    bool timedOut = false;
    IoResult result;
    result.size = 0;

    timer_.expires_after(OperationTimeout);
    timer_.async_wait(
      [this, &completed, &timedOut](
        const Wt::AsioWrapper::error_code& error) {
        if (!error && !completed) {
          timedOut = true;
          Wt::AsioWrapper::error_code ignored;
          socket_.close(ignored);
        }
      });

    start(
      [this, &completed, &result](
        const Wt::AsioWrapper::error_code& error, std::size_t size) {
        result.error = error;
        result.size = size;
        completed = true;
        timer_.cancel();
      });

    ioService_.run();

    if (timedOut) {
      throw std::runtime_error(operation + " timed out");
    }
    if (!completed) {
      throw std::runtime_error(operation + " did not complete");
    }

    return result;
  }

  static void requireSuccess(const IoResult& result,
                             std::size_t expectedSize,
                             const std::string& operation)
  {
    if (result.error) {
      throw Wt::AsioWrapper::system_error(result.error);
    }
    if (result.size != expectedSize) {
      throw std::runtime_error(operation + " transferred an unexpected size");
    }
  }
};

HttpResponse performHttpRequest(unsigned short port,
                                const std::string& request)
{
  TimedConnection connection(port);
  connection.write(request);
  return parseHttpResponse(connection.readUntilClose());
}

std::string extractSessionId(const std::string& body)
{
  const std::string marker = "?wtd=";
  const std::size_t markerPosition = body.find(marker);
  if (markerPosition == std::string::npos) {
    throw std::runtime_error("bootstrap response contains no session ID");
  }

  const std::size_t begin = markerPosition + marker.size();
  std::size_t end = begin;
  while (end < body.size() &&
         std::isalnum(static_cast<unsigned char>(body[end]))) {
    ++end;
  }
  if (end == begin) {
    throw std::runtime_error("bootstrap response contains an empty session ID");
  }

  return body.substr(begin, end - begin);
}

class TemporaryConfiguration
{
public:
  TemporaryConfiguration()
    : fileName_(Wt::FileUtils::createTempFileName())
  {
    if (fileName_.empty()) {
      throw std::runtime_error("could not create a temporary configuration");
    }

    try {
      std::ofstream config(fileName_.c_str(),
                           std::ios_base::out | std::ios_base::trunc);
      if (!config) {
        throw std::runtime_error("could not open the temporary configuration");
      }
      config << "<server>"
             << "<application-settings location=\"*\">"
             << "<web-sockets>true</web-sockets>"
             << "</application-settings>"
             << "</server>";
      config.close();
      if (!config) {
        throw std::runtime_error("could not write the temporary configuration");
      }
    } catch (...) {
      std::remove(fileName_.c_str());
      throw;
    }
  }

  ~TemporaryConfiguration()
  {
    std::remove(fileName_.c_str());
  }

  const std::string& fileName() const
  {
    return fileName_;
  }

private:
  TemporaryConfiguration(const TemporaryConfiguration&) = delete;
  TemporaryConfiguration& operator=(const TemporaryConfiguration&) = delete;

  std::string fileName_;
};

struct WebSocketFrame
{
  bool fin;
  unsigned int rsv;
  unsigned int opcode;
  bool masked;
  std::vector<unsigned char> payload;
};

class WebSocketClient
{
public:
  WebSocketClient(unsigned short port,
                  const std::string& host,
                  const std::string& sessionId)
    : connection_(port)
  {
    std::ostringstream request;
    request << "GET /?wtd=" << sessionId << "&request=ws HTTP/1.1\r\n"
            << "Host: " << host << "\r\n"
            << "Origin: http://" << host << "\r\n"
            << "Upgrade: websocket\r\n"
            << "Connection: Upgrade\r\n"
            << "Sec-WebSocket-Version: 13\r\n"
            << "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            << "Accept-Encoding: gzip\r\n"
            << "\r\n";
    connection_.write(request.str());
    response_ = connection_.readResponseHeaders();
  }

  const HttpResponse& response() const
  {
    return response_;
  }

  WebSocketFrame readFrame()
  {
    const std::vector<unsigned char> header = connection_.read(2);

    WebSocketFrame frame;
    frame.fin = (header[0] & 0x80) != 0;
    frame.rsv = (header[0] & 0x70) >> 4;
    frame.opcode = header[0] & 0x0f;
    frame.masked = (header[1] & 0x80) != 0;

    std::uint64_t payloadSize = header[1] & 0x7f;
    if (payloadSize == 126) {
      const std::vector<unsigned char> length = connection_.read(2);
      payloadSize = (static_cast<std::uint64_t>(length[0]) << 8)
        | static_cast<std::uint64_t>(length[1]);
    } else if (payloadSize == 127) {
      const std::vector<unsigned char> length = connection_.read(8);
      if ((length[0] & 0x80) != 0) {
        throw std::runtime_error("WebSocket payload length has its MSB set");
      }
      payloadSize = 0;
      for (std::size_t i = 0; i < length.size(); ++i) {
        payloadSize = (payloadSize << 8) | length[i];
      }
    }

    if (payloadSize > MaxFramePayloadSize ||
        payloadSize > std::numeric_limits<std::size_t>::max()) {
      throw std::runtime_error("WebSocket payload is unreasonably large");
    }

    std::array<unsigned char, 4> mask = {{ 0, 0, 0, 0 }};
    if (frame.masked) {
      const std::vector<unsigned char> maskBytes = connection_.read(4);
      std::copy(maskBytes.begin(), maskBytes.end(), mask.begin());
    }

    frame.payload = connection_.read(static_cast<std::size_t>(payloadSize));
    if (frame.masked) {
      for (std::size_t i = 0; i < frame.payload.size(); ++i) {
        frame.payload[i] ^= mask[i % mask.size()];
      }
    }

    return frame;
  }

  void sendText(const std::string& payload)
  {
    sendFrame(0x1, payload);
  }

  void sendClose()
  {
    sendFrame(0x8, std::string());
  }

private:
  TimedConnection connection_;
  HttpResponse response_;

  void sendFrame(unsigned int opcode, const std::string& payload)
  {
    if (payload.size() > 125) {
      throw std::runtime_error("test client payload is too large");
    }

    const std::array<unsigned char, 4> mask = {{ 0x37, 0xfa, 0x21, 0x3d }};
    std::vector<unsigned char> frame;
    frame.reserve(2 + mask.size() + payload.size());
    frame.push_back(static_cast<unsigned char>(0x80 | opcode));
    frame.push_back(static_cast<unsigned char>(0x80 | payload.size()));
    frame.insert(frame.end(), mask.begin(), mask.end());

    for (std::size_t i = 0; i < payload.size(); ++i) {
      frame.push_back(static_cast<unsigned char>(payload[i])
                      ^ mask[i % mask.size()]);
    }

    connection_.write(frame);
  }
};

class WebSocketFixture
{
public:
  WebSocketFixture()
    : server_(new Wt::WServer("test"))
  {
    const std::vector<std::string> arguments = {
      "--http-address", "127.0.0.1",
      "--http-port", "0",
      "--docroot", ".",
      "--config", configuration_.fileName()
    };
    server_->setServerConfiguration("test", arguments);
    server_->addEntryPoint(
      Wt::EntryPointType::Application,
      [](const Wt::WEnvironment& environment) {
        return std::make_unique<Wt::WApplication>(environment);
      });
    if (!server_->start()) {
      throw std::runtime_error("could not start the test server");
    }

    port_ = static_cast<unsigned short>(server_->httpPort());
    host_ = "127.0.0.1:" + std::to_string(port_);
  }

  std::unique_ptr<WebSocketClient> openWebSocket()
  {
    const std::string sessionId = bootstrapSession();
    return std::unique_ptr<WebSocketClient>(
      new WebSocketClient(port_, host_, sessionId));
  }

private:
  TemporaryConfiguration configuration_;
  std::unique_ptr<Wt::WServer> server_;
  unsigned short port_;
  std::string host_;

  std::string bootstrapSession()
  {
    std::ostringstream get;
    get << "GET / HTTP/1.1\r\n"
        << "Host: " << host_ << "\r\n"
        << "Connection: close\r\n"
        << "\r\n";
    const HttpResponse bootstrap = performHttpRequest(port_, get.str());
    if (bootstrap.status != 200) {
      throw std::runtime_error("bootstrap GET did not return HTTP 200");
    }
    const std::string sessionId = extractSessionId(bootstrap.body);

    std::ostringstream post;
    post << "POST /?wtd=" << sessionId
         << "&js=no&signal=load HTTP/1.1\r\n"
         << "Host: " << host_ << "\r\n"
         << "Content-Type: application/x-www-form-urlencoded\r\n"
         << "Content-Length: 0\r\n"
         << "Connection: close\r\n"
         << "\r\n";
    const HttpResponse load = performHttpRequest(port_, post.str());
    if (load.status != 200) {
      throw std::runtime_error("signal=load POST did not return HTTP 200");
    }

    return sessionId;
  }
};

void requireWebSocketHandshake(const HttpResponse& response)
{
  BOOST_REQUIRE_EQUAL(response.status, 101);
  BOOST_REQUIRE(containsToken(response.header("Connection"), "Upgrade"));
  BOOST_REQUIRE_EQUAL(lowerCase(response.header("Upgrade")), "websocket");
  BOOST_REQUIRE_EQUAL(response.header("Sec-WebSocket-Accept"),
                      "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
  BOOST_REQUIRE(!response.hasHeader("Content-Encoding"));
}

void requireWebSocketFrame(const WebSocketFrame& frame,
                           unsigned int opcode,
                           const std::string& payload)
{
  BOOST_REQUIRE(frame.fin);
  BOOST_REQUIRE_EQUAL(frame.rsv, 0u);
  BOOST_REQUIRE_EQUAL(frame.opcode, opcode);
  BOOST_REQUIRE(!frame.masked);
  BOOST_REQUIRE_EQUAL(std::string(frame.payload.begin(), frame.payload.end()),
                      payload);
}

} // namespace

BOOST_AUTO_TEST_CASE( websocket_upgrade_is_not_http_compressed )
{
  WebSocketFixture fixture;
  std::unique_ptr<WebSocketClient> client = fixture.openWebSocket();

  requireWebSocketHandshake(client->response());
  requireWebSocketFrame(client->readFrame(), 0x1, "connect");
}

BOOST_AUTO_TEST_CASE( websocket_message_and_close_frames )
{
  WebSocketFixture fixture;
  std::unique_ptr<WebSocketClient> client = fixture.openWebSocket();

  requireWebSocketHandshake(client->response());
  requireWebSocketFrame(client->readFrame(), 0x1, "connect");

  client->sendText("&signal=ping");
  requireWebSocketFrame(client->readFrame(), 0x1, "{}");

  client->sendClose();
  requireWebSocketFrame(client->readFrame(), 0x8, std::string());
}
