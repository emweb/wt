/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// bugfix for https://svn.boost.org/trac/boost/ticket/5722
#include <Wt/AsioWrapper/asio.hpp>

#include "Wt/Http/Client.h"
#include "Wt/WApplication.h"
#include "Wt/WIOService.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WServer.h"
#include "Wt/Utils.h"

#include "web/WebUtils.h"

#include <sstream>
#include <boost/algorithm/string.hpp>

#ifdef WT_WITH_SSL

#include <Wt/AsioWrapper/ssl.hpp>

#define VERIFY_CERTIFICATE

#ifdef WT_WIN32
#include "web/SslUtils.h"
#endif // WT_WIN32

#endif // WT_WITH_SSL

#ifdef WT_WIN32
#define strcasecmp _stricmp
#endif

using Wt::AsioWrapper::asio::ip::tcp;

namespace {
constexpr const int STATUS_NO_CONTENT = 204;
constexpr const int STATUS_MOVED_PERMANENTLY = 301;
constexpr const int STATUS_FOUND = 302;
constexpr const int STATUS_SEE_OTHER = 303;
constexpr const int STATUS_TEMPORARY_REDIRECT = 307;
}

namespace Wt {

namespace asio = AsioWrapper::asio;

LOGGER("Http.Client");

  namespace Http {

class Client::Impl : public std::enable_shared_from_this<Client::Impl>
{
public:
  struct ChunkState {
    enum class State { Size, Data, Complete, Error } state;
    std::size_t size;
    int parsePos;
  };

  Impl(asio::io_service& ioService, WServer *server,
       const std::string& sessionId)
    : ioService_(ioService),
      strand_(ioService),
      resolver_(ioService_),
      timer_(ioService_),
      server_(server),
      sessionId_(sessionId),
      timeout_(0),
      maximumResponseSize_(0),
      responseSize_(0),
      aborted_(false),
      headersOnly_(false)
  { }

  virtual ~Impl() { }

  void setTimeout(std::chrono::steady_clock::duration timeout) { 
    timeout_ = timeout; 
  }

  void setMaximumResponseSize(std::size_t bytes) {
    maximumResponseSize_ = bytes;
  }

  void request(const std::string& method, const std::string& protocol, const std::string& auth,
	       const std::string& server, int port, const std::string& path,
	       const Message& message)
  {
    headersOnly_ = (method == "HEAD");

    std::ostream request_stream(&requestBuf_);
    request_stream << method << " " << path << " HTTP/1.1\r\n";
    if ((protocol == "http" && port == 80) || (protocol == "https" && port == 443))
      request_stream << "Host: " << server << "\r\n";
    else
      request_stream << "Host: " << server << ":"
                     << std::to_string(port) << "\r\n";

    if (!auth.empty())
      request_stream << "Authorization: Basic " 
		     << Wt::Utils::base64Encode(auth) << "\r\n";

    bool haveContentLength = false;
    for (unsigned i = 0; i < message.headers().size(); ++i) {
      const Message::Header& h = message.headers()[i];
      if (strcasecmp(h.name().c_str(), "Content-Length") == 0)
	haveContentLength = true;
      request_stream << h.name() << ": " << h.value() << "\r\n";
    }

    if ((method == "POST" || method == "PUT" || method == "DELETE" || method == "PATCH") &&
	!haveContentLength)
      request_stream << "Content-Length: " << message.body().length() 
		     << "\r\n";

    request_stream << "\r\n";

    if (method == "POST" || method == "PUT" || method == "DELETE" || method == "PATCH")
      request_stream << message.body();

    tcp::resolver::query query(server, std::to_string(port));

    startTimer();
    resolver_.async_resolve
      (query,
       strand_.wrap(std::bind(&Impl::handleResolve,
			      shared_from_this(),
			      std::placeholders::_1,
			      std::placeholders::_2)));
  }

  void asyncStop(std::shared_ptr<Impl> *impl)
  {
    ioService_.post
      (strand_.wrap(std::bind(&Impl::stop, shared_from_this(), impl)));
  }

  Signal<AsioWrapper::error_code, Message>& done() { return done_; }
  Signal<Message>& headersReceived() { return headersReceived_; }
  Signal<std::string>& bodyDataReceived() { return bodyDataReceived_; }

  bool hasServer() { return server_ != nullptr; }

protected:
  typedef std::function<void(const AsioWrapper::error_code&)>
    ConnectHandler;
  typedef std::function<void(const AsioWrapper::error_code&,
			     const std::size_t&)> IOHandler;

  virtual tcp::socket& socket() = 0;
  virtual void asyncConnect(tcp::endpoint& endpoint,
			    const ConnectHandler& handler) = 0;
  virtual void asyncHandshake(const ConnectHandler& handler) = 0;
  virtual void asyncWriteRequest(const IOHandler& handler) = 0;
  virtual void asyncReadUntil(const std::string& s,
			      const IOHandler& handler) = 0;
  virtual void asyncRead(const IOHandler& handler) = 0;

private:
  void stop(std::shared_ptr<Impl> *impl)
  {
    /* Within strand */

    aborted_ = true;

    try {
      if (socket().is_open()) {
	AsioWrapper::error_code ignored_ec;
	socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
	socket().close();
      }
    } catch (std::exception& e) {
      LOG_INFO("Client::abort(), stop(), ignoring error: " << e.what());
    }

    if (impl)
      impl->reset();
  }

  void startTimer()
  {
    timer_.expires_from_now(timeout_);
    timer_.async_wait
      (strand_.wrap(std::bind(&Impl::timeout, shared_from_this(),
			      std::placeholders::_1)));
  }

  void cancelTimer()
  {
    /* Within strand */

    timer_.cancel();
  }

  void timeout(const AsioWrapper::error_code& e)
  {
    /* Within strand */

    if (e != asio::error::operation_aborted) {
      AsioWrapper::error_code ignored_ec;
      socket().shutdown(asio::ip::tcp::socket::shutdown_both,
			ignored_ec);

      err_ = asio::error::timed_out;
    }
  }

  void handleResolve(const AsioWrapper::error_code& err,
		     tcp::resolver::iterator endpoint_iterator)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      // Attempt a connection to the first endpoint in the list.
      // Each endpoint will be tried until we successfully establish
      // a connection.
      tcp::endpoint endpoint = *endpoint_iterator;

      startTimer();
      asyncConnect(endpoint,
		   strand_.wrap(std::bind(&Impl::handleConnect,
					  shared_from_this(),
					  std::placeholders::_1,
					  ++endpoint_iterator)));
    } else {
      if (aborted_)
        err_ = asio::error::operation_aborted;
      else
        err_ = err;
      complete();
    }
  }
 
  void handleConnect(const AsioWrapper::error_code& err,
		     tcp::resolver::iterator endpoint_iterator)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      // The connection was successful. Do the handshake (SSL only)
      startTimer();
      asyncHandshake
	(strand_.wrap(std::bind(&Impl::handleHandshake,
				shared_from_this(),
				std::placeholders::_1)));
    } else if (endpoint_iterator != tcp::resolver::iterator()) {
      // The connection failed. Try the next endpoint in the list.
      socket().close();

      handleResolve(AsioWrapper::error_code(), endpoint_iterator);
    } else {
      if (aborted_)
        err_ = asio::error::operation_aborted;
      else
        err_ = err;
      complete();
    }
  }

  void handleHandshake(const AsioWrapper::error_code& err)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      // The handshake was successful. Send the request.
      startTimer();
      asyncWriteRequest
	(strand_.wrap
	 (std::bind(&Impl::handleWriteRequest,
		      shared_from_this(),
		      std::placeholders::_1,
		      std::placeholders::_2)));
    } else {
      if (aborted_)
        err_ = asio::error::operation_aborted;
      else
        err_ = err;
      complete();
    }
  }

  void handleWriteRequest(const AsioWrapper::error_code& err,
			  const std::size_t&)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      // Read the response status line.
      startTimer();
      asyncReadUntil
	("\r\n",
	 strand_.wrap
	 (std::bind(&Impl::handleReadStatusLine,
		      shared_from_this(),
		      std::placeholders::_1,
		      std::placeholders::_2)));
    } else {
      if (aborted_)
        err_ = asio::error::operation_aborted;
      else
        err_ = err;
      complete();
    }
  }

  bool addResponseSize(std::size_t s)
  {
    responseSize_ += s;

    if (maximumResponseSize_ && responseSize_ > maximumResponseSize_) {
      err_ = asio::error::message_size;
      return false;
    }

    return true;
  }

  void handleReadStatusLine(const AsioWrapper::error_code& err,
			    const std::size_t& s)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      if (!addResponseSize(s)) {
        complete();
	return;
      }

      // Check that response is OK.
      std::istream response_stream(&responseBuf_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
#ifdef WT_ASIO_IS_BOOST_ASIO
	err_ = boost::system::errc::make_error_code
	  (boost::system::errc::protocol_error);
#else
	err_ = std::make_error_code(std::errc::protocol_error);
#endif
	complete();
	return;
      }

      LOG_DEBUG(status_code << " " << status_message);

      response_.setStatus(status_code);

      // Read the response headers, which are terminated by a blank line.
      startTimer();
      asyncReadUntil
	("\r\n\r\n",
	 strand_.wrap
	 (std::bind(&Impl::handleReadHeaders,
		      shared_from_this(),
		      std::placeholders::_1,
		      std::placeholders::_2)));
    } else {
      if (aborted_)
        err_ = asio::error::operation_aborted;
      else
        err_ = err;
      complete();
    }
  }

  void handleReadHeaders(const AsioWrapper::error_code& err,
			 const std::size_t& s)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      if (!addResponseSize(s)) {
        complete();
	return;
      }

      chunkedResponse_ = false;
      contentLength_ = -1;

      // Process the response headers.
      std::istream response_stream(&responseBuf_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r") {
	std::size_t i = header.find(':');
	if (i != std::string::npos) {
	  std::string name = boost::trim_copy(header.substr(0, i));
	  std::string value = boost::trim_copy(header.substr(i+1));
	  response_.addHeader(name, value);

	  if (boost::iequals(name, "Transfer-Encoding") &&
	      boost::iequals(value, "chunked")) {
	    chunkedResponse_ = true;
	    chunkState_.size = 0;
	    chunkState_.parsePos = 0;
	    chunkState_.state = ChunkState::State::Size;
	  } else if (!headersOnly_ &&
                     boost::iequals(name, "Content-Length")) {
	    std::stringstream ss(value);
	    ss >> contentLength_;
	  }
	}
      }
      
      if (headersReceived_.isConnected()) {
	if (server_)
	  server_->post(sessionId_,
			std::bind(&Impl::emitHeadersReceived,
				  shared_from_this()));
	else
	  emitHeadersReceived();
      }

      bool done = headersOnly_ || response_.status() == STATUS_NO_CONTENT;
      // Write whatever content we already have to output.
      if (responseBuf_.size() > 0) {
	std::stringstream ss;
	ss << &responseBuf_;
        done = addBodyText(ss.str());
      }

      if (!done) {
	// Start reading remaining data until EOF.
	startTimer();
	asyncRead(strand_.wrap
	    (std::bind(&Impl::handleReadContent,
		       shared_from_this(),
		       std::placeholders::_1,
		       std::placeholders::_2)));
      } else {
	complete();
      }
    } else {
      if (!aborted_)
        err_ = asio::error::operation_aborted;
      else
        err_ = err;
      complete();
    }
  }

  void handleReadContent(const AsioWrapper::error_code& err,
			 const std::size_t& s)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      if (!addResponseSize(s)) {
        complete();
	return;
      }

      std::stringstream ss;
      ss << &responseBuf_;

      bool done = addBodyText(ss.str());

      if (!done) {
	// Continue reading remaining data until EOF.
	startTimer();
	asyncRead
	  (strand_.wrap
	   (std::bind(&Impl::handleReadContent,
		      shared_from_this(),
		      std::placeholders::_1,
		      std::placeholders::_2)));
      } else {
	complete();
      }
    } else if (!aborted_
               && err != asio::error::eof
	       && err != asio::error::shut_down
	       && err != asio::error::bad_descriptor
	       && err != asio::error::operation_aborted
	       && err.value() != 335544539) {
      err_ = err;
      complete();
    } else {
      if (aborted_)
        err_ = asio::error::operation_aborted;
      complete();
    }
  }

  // Returns whether we're done (caller must call complete())
  bool addBodyText(const std::string& text)
  {
    if (chunkedResponse_) {
      chunkedDecode(text);
      if (chunkState_.state == ChunkState::State::Error) {
	protocolError();
	return true;
      } else if (chunkState_.state == ChunkState::State::Complete) {
	return true;
      } else
	return false;
    } else {
      if (maximumResponseSize_)
	response_.addBodyText(text);

      LOG_DEBUG("Data: " << text);
      haveBodyData(text);

      return (contentLength_ >= 0) &&
	(response_.body().size() >= contentLength_);
    }
  }

  void chunkedDecode(const std::string& text)
  {
    std::string::const_iterator pos = text.begin();
    while (pos != text.end()) {
      switch (chunkState_.state) {
      case ChunkState::State::Size: {
	unsigned char ch = *(pos++);

	switch (chunkState_.parsePos) {
	case -2:
	  if (ch != '\r') {
	    chunkState_.state = ChunkState::State::Error; return;
	  }

	  chunkState_.parsePos = -1;

	  break;
	case -1:
	  if (ch != '\n') {
	    chunkState_.state = ChunkState::State::Error; return;
	  }

	  chunkState_.parsePos = 0;
	  
	  break;
	case 0:
	  if (ch >= '0' && ch <= '9') {
	    chunkState_.size <<= 4;
	    chunkState_.size |= (ch - '0');
	  } else if (ch >= 'a' && ch <= 'f') {
	    chunkState_.size <<= 4;
	    chunkState_.size |= (10 + ch - 'a');
	  } else if (ch >= 'A' && ch <= 'F') {
	    chunkState_.size <<= 4;
	    chunkState_.size |= (10 + ch - 'A');
	  } else if (ch == '\r') {
	    chunkState_.parsePos = 2;
	  } else if (ch == ';') {
	    chunkState_.parsePos = 1;
	  } else {
	     chunkState_.state = ChunkState::State::Error; return;
	  }

	  break;
	case 1:
	  /* Ignoring extensions and syntax for now */
	  if (ch == '\r')
	    chunkState_.parsePos = 2;

	  break;
	case 2:
	  if (ch != '\n') {
	    chunkState_.state = ChunkState::State::Error; return;
	  }

	  if (chunkState_.size == 0) {
	    chunkState_.state = ChunkState::State::Complete; return;
	  }
	    
	  chunkState_.state = ChunkState::State::Data;
	}

	break;
      }
      case ChunkState::State::Data: {
	std::size_t thisChunk
	  = std::min(std::size_t(text.end() - pos), chunkState_.size);
	std::string text = std::string(pos, pos + thisChunk);
	if (maximumResponseSize_)
	  response_.addBodyText(text);

	LOG_DEBUG("Chunked data: " << text);
	haveBodyData(text);
	chunkState_.size -= thisChunk;
	pos += thisChunk;

	if (chunkState_.size == 0) {
	  chunkState_.parsePos = -2;
	  chunkState_.state = ChunkState::State::Size;
	}
	break;
      }
      default:
	assert(false); // Illegal state
      }
    }
  }

  void protocolError()
  {
#ifdef WT_ASIO_IS_BOOST_ASIO
    err_ = boost::system::errc::make_error_code
      (boost::system::errc::protocol_error);
#else
    err_ = std::make_error_code(std::errc::protocol_error);
#endif
  } 

  void complete()
  {
    stop(nullptr);
    if (server_)
      server_->post(sessionId_,
		    std::bind(&Impl::emitDone, shared_from_this()));
    else
      emitDone();
  }

  void haveBodyData(std::string text)
  {
    if (bodyDataReceived_.isConnected()) {
      if (server_)
	server_->post(sessionId_,
		      std::bind(&Impl::emitBodyReceived, shared_from_this(),
				  text));
      else
	emitBodyReceived(text);
    }
  }

  void emitDone()
  {
    done_.emit(err_, response_);
  }

  void emitHeadersReceived() {
    headersReceived_.emit(response_);
  }

  void emitBodyReceived(std::string text) {
    bodyDataReceived_.emit(text);
  }

protected:
  asio::io_service& ioService_;
  AsioWrapper::strand strand_;
  tcp::resolver resolver_;
  asio::streambuf requestBuf_;
  asio::streambuf responseBuf_;

private:
  asio::steady_timer timer_;
  WServer *server_;
  std::string sessionId_;
  std::chrono::steady_clock::duration timeout_;
  std::size_t maximumResponseSize_, responseSize_;
  bool chunkedResponse_;
  ChunkState chunkState_;
  int contentLength_;
  AsioWrapper::error_code err_;
  Message response_;
  Signal<AsioWrapper::error_code, Message> done_;
  Signal<Message> headersReceived_;
  Signal<std::string> bodyDataReceived_;
  bool aborted_;
  bool headersOnly_;
};

class Client::TcpImpl final : public Client::Impl
{
public:
  TcpImpl(asio::io_service& ioService, WServer *server,
	  const std::string& sessionId)
    : Impl(ioService, server, sessionId),
      socket_(ioService_)
  { }

protected:
  virtual tcp::socket& socket() override
  {
    return socket_;
  }

  virtual void asyncConnect(tcp::endpoint& endpoint,
			    const ConnectHandler& handler) override
  {
    socket_.async_connect(endpoint, handler);
  }

  virtual void asyncHandshake(const ConnectHandler& handler) override
  {
    handler(AsioWrapper::error_code());
  }

  virtual void asyncWriteRequest(const IOHandler& handler) override
  {
    asio::async_write(socket_, requestBuf_, handler);
  }

  virtual void asyncReadUntil(const std::string& s,
			      const IOHandler& handler) override
  {
    asio::async_read_until(socket_, responseBuf_, s, handler);
  }

  virtual void asyncRead(const IOHandler& handler) override
  {
    asio::async_read(socket_, responseBuf_,
                            asio::transfer_at_least(1), handler);
  }

private:
  tcp::socket socket_;
};

#ifdef WT_WITH_SSL

class Client::SslImpl final : public Client::Impl
{
public:
  SslImpl(asio::io_service& ioService, bool verifyEnabled,
	  WServer *server,
	  asio::ssl::context& context, const std::string& sessionId,
	  const std::string& hostName)
    : Impl(ioService, server, sessionId),
      socket_(ioService_, context),
      verifyEnabled_(verifyEnabled),
      hostName_(hostName)
  {
#ifndef OPENSSL_NO_TLSEXT
    if (!SSL_set_tlsext_host_name(socket_.native_handle(), hostName.c_str())) {
      LOG_ERROR("could not set tlsext host.");
    }
#endif
  }

protected:
  virtual tcp::socket& socket() override
  {
    return socket_.next_layer();
  }

  virtual void asyncConnect(tcp::endpoint& endpoint,
			    const ConnectHandler& handler) override
  {
    socket_.lowest_layer().async_connect(endpoint, handler);
  }

  virtual void asyncHandshake(const ConnectHandler& handler) override
  {
#ifdef VERIFY_CERTIFICATE
    if (verifyEnabled_) {
      socket_.set_verify_mode(asio::ssl::verify_peer);
      LOG_DEBUG("verifying that peer is " << hostName_);
      socket_.set_verify_callback
        (asio::ssl::rfc2818_verification(hostName_));
    }
#endif // VERIFY_CERTIFICATE
    socket_.async_handshake(asio::ssl::stream_base::client, handler);
  }

  virtual void asyncWriteRequest(const IOHandler& handler) override
  {
    asio::async_write(socket_, requestBuf_, handler);
  }

  virtual void asyncReadUntil(const std::string& s,
			      const IOHandler& handler) override
  {
    asio::async_read_until(socket_, responseBuf_, s, handler);
  }

  virtual void asyncRead(const IOHandler& handler) override
  {
    asio::async_read(socket_, responseBuf_,
                            asio::transfer_at_least(1), handler);
  }

private:
  typedef asio::ssl::stream<tcp::socket> ssl_socket;

  ssl_socket socket_;
  bool verifyEnabled_;
  std::string hostName_;
};
#endif // WT_WITH_SSL

Client::Client()
  : ioService_(0),
    timeout_(std::chrono::seconds{10}),
    maximumResponseSize_(64*1024),
#ifdef VERIFY_CERTIFICATE
    verifyEnabled_(true),
#else
    verifyEnabled_(false),
#endif
    followRedirect_(false),
    redirectCount_(0),
    maxRedirects_(20)
{ }

Client::Client(asio::io_service& ioService)
  : ioService_(&ioService),
    timeout_(std::chrono::seconds{10}),
    maximumResponseSize_(64*1024),
#ifdef VERIFY_CERTIFICATE
    verifyEnabled_(true),
#else
    verifyEnabled_(false),
#endif
    followRedirect_(false),
    redirectCount_(0),
    maxRedirects_(20)
{ }

Client::~Client()
{
  abort();
}

void Client::setSslCertificateVerificationEnabled(bool enabled)
{
  verifyEnabled_ = enabled;
}

void Client::abort()
{
  std::shared_ptr<Impl> impl = impl_;
  if (impl) {
    if (impl->hasServer()) {
      // handling of redirect happens in the WApplication
      impl->asyncStop(nullptr);
      impl_.reset();
    } else {
      // handling of redirect happens in the strand of impl
      impl->asyncStop(&impl_);
    }
  }
}

void Client::setTimeout(std::chrono::steady_clock::duration timeout)
{
  timeout_ = timeout;
}

void Client::setMaximumResponseSize(std::size_t bytes)
{
  maximumResponseSize_ = bytes;
}

void Client::setSslVerifyFile(const std::string& file)
{
  verifyFile_ = file;
}

void Client::setSslVerifyPath(const std::string& path)
{
  verifyPath_ = path;
}

bool Client::get(const std::string& url)
{
  return request(Http::Method::Get, url, Message());
}

bool Client::get(const std::string& url, 
		 const std::vector<Message::Header> headers)
{
  Message m(headers);
  return request(Http::Method::Get, url, m);
}

bool Client::head(const std::string& url)
{
  return request(Http::Method::Head, url, Message());
}

bool Client::head(const std::string& url,
                  const std::vector<Message::Header> headers)
{
  Message m(headers);
  return request(Http::Method::Head, url, m);
}

bool Client::post(const std::string& url, const Message& message)
{
  return request(Http::Method::Post, url, message);
}

bool Client::put(const std::string& url, const Message& message)
{
  return request(Http::Method::Put, url, message);
}

bool Client::deleteRequest(const std::string& url, const Message& message)
{
  return request(Http::Method::Delete, url, message);
}

bool Client::patch(const std::string& url, const Message& message)
{
  return request(Http::Method::Patch, url, message);
}

bool Client::request(Http::Method method, const std::string& url,
		     const Message& message)
{
  std::string sessionId;

  asio::io_service *ioService = ioService_;
  WServer *server = nullptr;

  WApplication *app = WApplication::instance();

  if(impl_.get()) {
    LOG_ERROR("another request is in progress");
    return false;
  }

  if (app && !ioService) {
    sessionId = app->sessionId();
    server = app->environment().server();
    ioService = &server->ioService();
  } else if (!ioService) {
    server = WServer::instance();

    if (server)
      ioService = &server->ioService();
    else {
      LOG_ERROR("requires a WIOService for async I/O");
      return false;
    }

    server = nullptr;
  }

  URL parsedUrl;

  if (!parseUrl(url, parsedUrl))
    return false;

  if (parsedUrl.protocol == "http") {
    impl_.reset(new TcpImpl(*ioService, server, sessionId));

#ifdef WT_WITH_SSL
  } else if (parsedUrl.protocol == "https") {
#if defined(WT_ASIO_IS_BOOST_ASIO) && BOOST_VERSION >= 106600
    asio::ssl::context context(asio::ssl::context::tls);
#elif defined(WT_ASIO_IS_STANDALONE_ASIO) && ASIO_VERSION >= 101100
    asio::ssl::context context(asio::ssl::context::sslv23);
#else
    asio::ssl::context context
      (*ioService, asio::ssl::context::sslv23);
#endif
    long sslOptions = asio::ssl::context::no_sslv2 |
                      asio::ssl::context::no_sslv3 |
                      asio::ssl::context::no_tlsv1;

#if (defined(WT_ASIO_IS_BOOST_ASIO) && BOOST_VERSION >= 105800) || \
     defined(WT_ASIO_IS_STANDALONE_ASIO)
    sslOptions |= asio::ssl::context::no_tlsv1_1;
#endif

    context.set_options(sslOptions);

#ifdef VERIFY_CERTIFICATE
    if (verifyEnabled_) {
      context.set_default_verify_paths();
#ifdef WT_WIN32
      Ssl::addWindowsCACertificates(context);
#endif // WT_WIN32
    }

    if (!verifyFile_.empty() || !verifyPath_.empty()) {
      if (!verifyFile_.empty())
	context.load_verify_file(verifyFile_);
      if (!verifyPath_.empty())
	context.add_verify_path(verifyPath_);
    }
#endif // VERIFY_CERTIFICATE

    impl_.reset(new SslImpl(*ioService, verifyEnabled_,
			    server, 
			    context, 
			    sessionId, 
			    parsedUrl.host));
#endif // WT_WITH_SSL

  } else {
    LOG_ERROR("unsupported protocol: " << parsedUrl.protocol);
    return false;
  }

  if (followRedirect()) {
    impl_->done().connect(std::bind(&Client::handleRedirect,
				    this, method, std::placeholders::_1,
				    std::placeholders::_2, message));
  } else {
    impl_->done().connect(this, &Client::emitDone);
  }

  if (headersReceived_.isConnected())
    impl_->headersReceived().connect(this, &Client::emitHeadersReceived);

  if (bodyDataReceived_.isConnected())
    impl_->bodyDataReceived().connect(this, &Client::emitBodyReceived);

  impl_->setTimeout(timeout_);
  impl_->setMaximumResponseSize(maximumResponseSize_);

  const char *methodNames_[] = { "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD" };

  LOG_DEBUG(methodNames_[static_cast<unsigned int>(method)] << " " << url);

  impl_->request(methodNames_[static_cast<unsigned int>(method)], 
		 parsedUrl.protocol,
		 parsedUrl.auth,
		 parsedUrl.host, 
		 parsedUrl.port, 
		 parsedUrl.path, 
		 message);

  return true;
}

bool Client::followRedirect() const
{
  return followRedirect_;
}

void Client::setFollowRedirect(bool followRedirect)
{
  followRedirect_ = followRedirect;
}

int Client::maxRedirects() const
{
  return maxRedirects_;
}

void Client::setMaxRedirects(int maxRedirects)
{
  maxRedirects_ = maxRedirects;
}

void Client::handleRedirect(Http::Method method,
			    AsioWrapper::error_code err,
			    const Message& response, const Message& request)
{
  if (!impl_) {
    emitDone(err, response);
    return;
  }
  impl_.reset();
  int status = response.status();
  if (!err && (((status == STATUS_MOVED_PERMANENTLY ||
                 status == STATUS_FOUND ||
                 status == STATUS_TEMPORARY_REDIRECT) && method == Http::Method::Get) ||
               status == STATUS_SEE_OTHER)) {
    const std::string *newUrl = response.getHeader("Location");
    ++ redirectCount_;
    if (newUrl) {
      if (redirectCount_ <= maxRedirects_) {
	get(*newUrl, request.headers());
	return;
      } else {
	LOG_WARN("Redirect count of " << maxRedirects_ 
		 << " exceeded! Redirect URL: " << *newUrl);
      }
    }
  }
  emitDone(err, response);
}

void Client::emitDone(AsioWrapper::error_code err, const Message& response)
{
  impl_.reset();
  redirectCount_ = 0;
  done_.emit(err, response);
}

void Client::emitHeadersReceived(const Message& response)
{
  headersReceived_.emit(response);
}

void Client::emitBodyReceived(const std::string& data)
{
  bodyDataReceived_.emit(data);
}

bool Client::parseUrl(const std::string &url, URL &parsedUrl)
{
  std::size_t i = url.find("://");
  if (i == std::string::npos) {
    LOG_ERROR("ill-formed URL: " << url);
    return false;
  }

  parsedUrl.protocol = url.substr(0, i);
  std::string rest = url.substr(i + 3);
  // find auth
  std::size_t l = rest.find('@');
  // find host
  std::size_t j = rest.find('/');
  if (l != std::string::npos &&
      (j == std::string::npos || j > l)) {
    // above check: userinfo can not contain a forward slash
    // path may contain @ (issue #7272)
    parsedUrl.auth = rest.substr(0, l);
    parsedUrl.auth = Wt::Utils::urlDecode(parsedUrl.auth);
    rest = rest.substr(l+1);
    if (j != std::string::npos) {
      j -= l + 1;
    }
  }

  if (j == std::string::npos) {
    parsedUrl.host = rest;
    parsedUrl.path = "/";
  } else {
    parsedUrl.host = rest.substr(0, j);
    parsedUrl.path = rest.substr(j);
  }

  std::size_t k = parsedUrl.host.find(':');
  if (k != std::string::npos) {
    try {
      parsedUrl.port = Utils::stoi(parsedUrl.host.substr(k + 1));
    } catch (std::exception& e) {
      LOG_ERROR("invalid port: " << parsedUrl.host.substr(k + 1));
      return false;
    }
    parsedUrl.host = parsedUrl.host.substr(0, k);
  } else {
    if (parsedUrl.protocol == "http")
      parsedUrl.port = 80;
    else if (parsedUrl.protocol == "https")
      parsedUrl.port = 443;
    else
      parsedUrl.port = 80; // protocol will not be handled anyway
  }

  return true;
}

  }
}
