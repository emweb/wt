/*
 * Copyright (C) 2014 Emweb bvba, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "ProxyReply.h"

#include "Wt/Http/Request"
#include "Connection.h"
#include "Server.h"
#include "StockReply.h"

namespace Wt {
  LOGGER("wthttp/proxy");
}

namespace http {
namespace server {

ProxyReply::ProxyReply(Request& request,
		       const Configuration& config,
		       SessionProcessManager& sessionManager)
  : Reply(request, config),
    sessionManager_(sessionManager),
    out_(&out_buf_),
    sending_(0),
    more_(true),
    receiving_(false)
{
  reset(0);
}

ProxyReply::~ProxyReply()
{
  if (sessionProcess_ && sessionProcess_->sessionId().empty()) {
    sessionProcess_->stop();
  }
}

void ProxyReply::reset(const Wt::EntryPoint *ep)
{
  if (sessionProcess_ && sessionProcess_->sessionId().empty()) {
    sessionProcess_->stop();
  }
  sessionProcess_.reset();
  socket_.reset();
  contentType_.clear();
  requestBuf_.consume(requestBuf_.size());
  responseBuf_.consume(responseBuf_.size());
  out_buf_.consume(out_buf_.size());
  sending_ = 0;
  more_ = true;
  receiving_ = false;

  Reply::reset(ep);
}

void ProxyReply::writeDone(bool success)
{
  if (!success) {
    return;
  }

  out_buf_.consume(sending_);

  if (request_.type == Request::TCP && !receiving_) {
    // Sent 101 response, start receiving more data from the client
    receiving_ = true;
    receive();
  }

  if (more_) {
    // Get more data to send
    asio::async_read(*socket_, responseBuf_,
	asio::transfer_at_least(1),
	connection()->strand().wrap(
	  boost::bind(&ProxyReply::handleResponseRead,
	    boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	    asio::placeholders::error)));
  }
}

bool ProxyReply::consumeData(Buffer::const_iterator begin,
			     Buffer::const_iterator end,
			     Request::State state)
{
  if (state == Request::Error) {
    socket_.reset();
    return false;
  }

  beginRequestBuf_ = begin;
  endRequestBuf_ = end;
  state_ = state;

  if (sessionProcess_) {
    // Connection with child already established, send request data
    asio::async_write(*socket_, asio::buffer(beginRequestBuf_, endRequestBuf_ - beginRequestBuf_),
	  boost::bind(&ProxyReply::handleDataWritten,
	    boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	    asio::placeholders::error,
	    asio::placeholders::bytes_transferred));
  } else {
    // First connect to child process
    std::string sessionId = getSessionId();

    sessionProcess_ = sessionManager_.sessionProcess(sessionId);

    if (sessionId.empty() || !sessionProcess_) {
      if (sessionManager_.tryToIncrementSessionCount()) {
	// Launch new child process
	sessionProcess_.reset(
	    new SessionProcess(connection()->server()->service()));

	sessionProcess_->asyncExec(
	    configuration(),
	    boost::bind(&ProxyReply::connectToChild,
	      boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()), _1));
      } else {
	LOG_ERROR("maximum amount of sessions reached!");
	error(service_unavailable);
      }
    } else {
      connectToChild(true);
    }
  }

  // Don't immediately consume more data, but do this when it has been sent to child
  return false;
}

void ProxyReply::connectToChild(bool success)
{
  if (success) {
    socket_.reset(new asio::ip::tcp::socket(connection()->server()->service()));
    socket_->async_connect(
	sessionProcess_->endpoint(),
	boost::bind(&ProxyReply::handleChildConnected,
	    boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	    asio::placeholders::error));
  } else {
    error(service_unavailable);
  }
}


void ProxyReply::handleChildConnected(const boost::system::error_code& ec)
{
  if (ec) {
    LOG_ERROR("error connecting to child: " << ec.message());
    error(service_unavailable);
    return;
  }

  assembleRequestHeaders();

  // Send any request data we already have
  std::ostream os(&requestBuf_);
  os.write(beginRequestBuf_, static_cast<std::streamsize>(endRequestBuf_ - beginRequestBuf_));

  asio::async_write(*socket_, requestBuf_,
      boost::bind(&ProxyReply::handleDataWritten,
	boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	asio::placeholders::error,
	asio::placeholders::bytes_transferred));
}

void ProxyReply::assembleRequestHeaders()
{
  std::ostream os(&requestBuf_);
  os << request_.method.data << " " << request_.uri.data << " HTTP/1.1\r\n";
  bool establishWebSockets = false;
  std::string forwardedFor;
  for (Request::HeaderList::const_iterator it = request_.headers.begin();
       it != request_.headers.end(); ++it) {
    if (it->name.iequals("Connection") || it->name.iequals("Keep-Alive") ||
	it->name.iequals("TE") || it->name.iequals("Transfer-Encoding")) {
      // Remove hop-by-hop header
    } else if (it->name.iequals("X-Forwarded-For") || it->name.iequals("Client-IP")) {
      const Wt::Configuration& wtConfiguration = connection()->server()->controller()->configuration();
      if (wtConfiguration.behindReverseProxy()) {
	forwardedFor = std::string(it->value.data) + ", ";
      }
    } else if (it->name.iequals("Upgrade")) {
      if (it->value.iequals("websocket")) {
	establishWebSockets = true;
      }
    } else {
      os << it->name.data << ": " << it->value.data << "\r\n";
    }
  }
  if (establishWebSockets) {
    os << "Connection: Upgrade\r\n";
    os << "Upgrade: websocket\r\n";
  } else {
    os << "Connection: close\r\n";
  }
  os << "X-Forwarded-For: " << forwardedFor << request_.remoteIP << "\r\n";
  os << "\r\n";
}

void ProxyReply::handleDataWritten(const boost::system::error_code &ec,
				   std::size_t transferred)
{
  if (!ec) {
    if (state_ == Request::Partial) {
      requestBuf_.consume(transferred);
      receive();
    } else {
      asio::async_read_until(*socket_, responseBuf_, "\r\n",
	  boost::bind(&ProxyReply::handleStatusRead,
	    boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	    asio::placeholders::error));
    }
  } else {
    LOG_ERROR("error sending data to child: " << ec.message());
    error(service_unavailable);
  }
}

void ProxyReply::handleStatusRead(const boost::system::error_code &ec)
{
  if (!ec) {
    std::istream response_stream(&responseBuf_);
    std::string http_version;
    response_stream >> http_version;
    int status_code;
    response_stream >> status_code;
    setStatus((Reply::status_type) status_code);
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
      LOG_ERROR("got malformed response!");
      error(internal_server_error);
      return;
    }

    asio::async_read_until(*socket_, responseBuf_, "\r\n\r\n",
	boost::bind(&ProxyReply::handleHeadersRead,
	  boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	  asio::placeholders::error));
  } else {
    LOG_ERROR("error reading status line: " << ec.message());
    error(service_unavailable);
  }
}

void ProxyReply::handleHeadersRead(const boost::system::error_code &ec)
{
  if (ec) {
    LOG_ERROR("error reading headers: " << ec.message());
    error(service_unavailable);
    return;
  }

  bool webSocketStatus = status() == switching_protocols;
  bool webSocketConnection = false;
  bool webSocketUpgrade = false;

  std::istream response_stream(&responseBuf_);
  std::string header;

  while (std::getline(response_stream, header) && header != "\r") {
    std::size_t i = header.find(':');
    if (i != std::string::npos) {
      std::string name = boost::trim_copy(header.substr(0, i));
      std::string value = boost::trim_copy(header.substr(i+1));
      if (boost::iequals(name, "Content-Type")) {
	contentType_ = value;
      } else if (boost::iequals(name, "Date")) {
	// Ignore, we're overriding it
      } else if (boost::iequals(name, "Transfer-Encoding") || boost::iequals(name, "Keep-Alive") ||
	  boost::iequals(name, "TE")) {
	// Remove hop-by-hop header
      } else if (boost::iequals(name, "Connection")) {
	// Remove hop-by-hop header
	if (boost::icontains(value, "Upgrade")) {
	  webSocketConnection = true;
	}
      } else if (boost::iequals(name, "X-Wt-Session")) {
	sessionManager_.addSessionProcess(value, sessionProcess_);
      } else if (boost::iequals(name, "Upgrade")) {
	// Remove hop-by-hop header
	if (boost::icontains(value, "websocket")) {
	  webSocketUpgrade = true;
	}
      } else {
	addHeader(name, value);
      }

      if (boost::iequals(name, "Transfer-Encoding") &&
	  boost::iequals(value, "chunked")) {
	// NOTE: Wt shouldn't return chunked encoding, because
	//	 we sent Connection: close.
	// If decoding is needed, look in Wt::Http::Client
	LOG_ERROR("unexpected chunked encoding!");
	error(internal_server_error);
	return;
      }
    }
  }

  if (webSocketStatus && webSocketConnection && webSocketUpgrade) {
    addHeader("Connection", "Upgrade");
    addHeader("Upgrade", "websocket");
    setCloseConnection();
    request_.type = Request::TCP;
  }

  if (responseBuf_.size() > 0) {
    out_ << &responseBuf_;
  }

  send();
}

void ProxyReply::handleResponseRead(const boost::system::error_code &ec)
{
  if (!ec) {
    if (responseBuf_.size() > 0) {
      out_ << &responseBuf_;
    }

    send();
  } else if (ec == boost::asio::error::eof
	     || ec == boost::asio::error::shut_down
	     || ec == boost::asio::error::operation_aborted
	     || ec == boost::asio::error::connection_reset) {
    more_ = false;

    if (request_.type != Request::TCP) {
      send();
    }
  } else {
    LOG_ERROR("error reading response: " << ec.message());
    error(service_unavailable);
  }
}

std::string ProxyReply::getSessionId() const
{
  std::string sessionId;
  Wt::Http::ParameterMap params;
  Wt::Http::Request::parseFormUrlEncoded(request_.request_query, params);
  std::string wtd;
  if (params.find("wtd") != params.end()) {
    wtd = params["wtd"][0];
  }
  const Wt::Configuration& wtConfiguration = connection()->server()->controller()->configuration();
  if (wtConfiguration.sessionTracking() == Wt::Configuration::CookiesURL
      && !wtConfiguration.reloadIsNewSession()) {
    const Request::Header *cookieHeader = request_.getHeader("Cookie");
    if (cookieHeader) {
      sessionId = Wt::WebController::sessionFromCookie(cookieHeader->value.data,
							 request_.request_path,
							 wtConfiguration.sessionIdLength());
    }
  }

  if (sessionId.empty() && !wtd.empty())
    sessionId = wtd;

  return sessionId;
}

std::string ProxyReply::contentType()
{
  return contentType_;
}

::int64_t ProxyReply::contentLength()
{
  return -1;
}

bool ProxyReply::nextContentBuffers(std::vector<asio::const_buffer>& result)
{
  sending_ = out_buf_.size();
  if (sending_ > 0) {
    result.push_back(out_buf_.data());
  }
  return !more_;
}

void ProxyReply::error(status_type status)
{
  if (request_.type == Request::HTTP) {
    setStatus(status);
    setCloseConnection();
    more_ = false;
    setRelay(ReplyPtr(new StockReply(request_, status, configuration())));
    send();
  } else {
    connection()->close();
  }
}

} // namespace server
} // namespace http
