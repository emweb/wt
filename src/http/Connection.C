/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
// 
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define DEBUG

#include <vector>

#include "Connection.h"
#include "ConnectionManager.h"
#include "RequestHandler.h"
#include "StockReply.h"
#include "Server.h"
#include "WebController.h"

namespace Wt {
  LOGGER("wthttp/async");
}

#if BOOST_VERSION >= 104900
typedef std::chrono::seconds asio_timer_seconds;
#else
typedef boost::posix_time::seconds asio_timer_seconds;
#endif

namespace http {
namespace server {

static const int CONNECTION_TIMEOUT = 300; // 5 minutes
static const int BODY_TIMEOUT = 600;       // 10 minutes
static const int KEEPALIVE_TIMEOUT  = 10;  // 10 seconds

Connection::Connection(asio::io_service& io_service, Server *server,
    ConnectionManager& manager, RequestHandler& handler)
  : ConnectionManager_(manager),
    strand_(io_service),
    state_(Idle),
    request_handler_(handler),
    readTimer_(io_service),
    writeTimer_(io_service),
    request_parser_(server),
    server_(server),
    waitingResponse_(false),
    haveResponse_(false),
    responseDone_(false)
{ }

Connection::~Connection()
{
  LOG_DEBUG("~Connection");
}

#if (defined(WT_ASIO_IS_BOOST_ASIO) && BOOST_VERSION >= 106600) || (defined(WT_ASIO_IS_STANDALONE_ASIO) && ASIO_VERSION >= 101100)
asio::ip::tcp::socket::native_handle_type Connection::native()
{
  return socket().native_handle();
}
#else
asio::ip::tcp::socket::native_type Connection::native()
{
  return socket().native();
}
#endif

void Connection::finishReply()
{ 
  if (!request_.uri.empty()) {
    LOG_DEBUG("last request: " << request_.method.str()
	      << " " << request_.uri.str()
	      << " (ws:" << request_.webSocketVersion << ")");
  }
}

void Connection::scheduleStop()
{
  server_->service()
    .post(strand_.wrap(std::bind(&Connection::stop, shared_from_this())));
}

void Connection::start()
{
  LOG_DEBUG(native() << ": start()");

  request_parser_.reset();
  request_.reset();
  try {
    request_.remoteIP = socket().remote_endpoint().address().to_string();
    request_.port = socket().local_endpoint().port();
  } catch (std::exception& e) {
    LOG_ERROR("remote_endpoint() threw: " << e.what());
  }

  Wt::AsioWrapper::error_code ignored_ec;
  socket().set_option(asio::ip::tcp::no_delay(true), ignored_ec);

  rcv_buffers_.push_back(Buffer());
  startAsyncReadRequest(rcv_buffers_.back(), CONNECTION_TIMEOUT);
}

void Connection::stop()
{
  lastWtReply_.reset();
  lastProxyReply_.reset();
  lastStaticReply_.reset();
}

void Connection::setReadTimeout(int seconds)
{
  if (seconds != 0) {
    LOG_DEBUG(native() << " setting read timeout (ws: "
	      << request_.webSocketVersion << ")");
    state_ |= Reading;

    readTimer_.expires_from_now(asio_timer_seconds(seconds));
    readTimer_.async_wait(std::bind(&Connection::timeout, shared_from_this(),
				    std::placeholders::_1));
  }
}

void Connection::setWriteTimeout(int seconds)
{
  LOG_DEBUG(native() << " setting write timeout (ws: "
	    << request_.webSocketVersion << ")");
  state_ |= Writing;

  writeTimer_.expires_from_now(asio_timer_seconds(seconds));
  writeTimer_.async_wait(std::bind(&Connection::timeout, shared_from_this(),
				   std::placeholders::_1));
}

void Connection::cancelReadTimer()
{
  LOG_DEBUG(native() << " cancel read timeout");
  state_.clear(Reading);

  readTimer_.cancel();
}

void Connection::cancelWriteTimer()
{
  LOG_DEBUG(native() << " cancel write timeout");
  state_.clear(Writing);

  writeTimer_.cancel();
}

void Connection::timeout(const Wt::AsioWrapper::error_code& e)
{
  if (e != asio::error::operation_aborted)
    strand_.post(std::bind(&Connection::doTimeout, shared_from_this()));
}

void Connection::doTimeout()
{
  Wt::AsioWrapper::error_code ignored_ec;
  socket().shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
  readTimer_.cancel();
  writeTimer_.cancel();
}

void Connection::handleReadRequest0()
{
  Buffer& buffer = rcv_buffers_.back();

#ifdef DEBUG
  try {
    LOG_DEBUG(socket().native_handle() << "incoming request: "
	      << socket().remote_endpoint().port() << " (avail= "
	      << (rcv_buffer_size_ - (rcv_remaining_ - buffer.data())) << "): "
	      << std::string(rcv_remaining_,
			     std::min((unsigned long)(buffer.data()
				      - rcv_remaining_ + rcv_buffer_size_),
				      (long unsigned)1000)));
  } catch (...) {
  }
#endif // DEBUG

  boost::tribool result;
  boost::tie(result, rcv_remaining_)
    = request_parser_.parse(request_,
			    &*rcv_remaining_, buffer.data() + rcv_buffer_size_);

  if (result) {
    Reply::status_type status = request_parser_.validate(request_);
    // FIXME: Let the reply decide whether we're doing websockets, move this logic to WtReply
    bool doWebSockets = server_->controller()->configuration().webSockets() &&
			(server_->controller()->configuration().sessionPolicy() != Wt::Configuration::DedicatedProcess ||
			 server_->configuration().parentPort() != -1);

    if (doWebSockets)
      request_.enableWebSocket();

    LOG_DEBUG(native() << "request: " << status);

    if (status >= 300)
      sendStockReply(status);
    else {
      if (request_.webSocketVersion >= 0) {
	// replace 'http' with 'ws'
	request_.urlScheme[0] = 'w';
	request_.urlScheme[1] = 's';
	strncpy(request_.urlScheme + 2, urlScheme() + 4, 7);
	request_.urlScheme[9] = 0;
      } else
        strncpy(request_.urlScheme, urlScheme(), 9);

      ReplyPtr reply;
      try {
	reply = request_handler_.handleRequest
	  (request_, lastWtReply_, lastProxyReply_, lastStaticReply_);
	reply->setConnection(shared_from_this());
      } catch (Wt::AsioWrapper::system_error& e) {
	LOG_ERROR("Error in handleRequest0(): " << e.what());
	handleError(e.code());
	return;
      }

      rcv_body_buffer_ = false;
      handleReadBody(reply);
    }
  } else if (!result) {
    sendStockReply(StockReply::bad_request);
  } else {
    rcv_buffers_.push_back(Buffer());
    startAsyncReadRequest(rcv_buffers_.back(), 
			  request_parser_.initialState()
			  ? KEEPALIVE_TIMEOUT 
			  : CONNECTION_TIMEOUT);
  }
}

void Connection::sendStockReply(StockReply::status_type status)
{
  ReplyPtr reply
    (new StockReply(request_, status, "", server_->configuration()));

  reply->setConnection(shared_from_this());
  reply->setCloseConnection();

  startWriteResponse(reply);
}

void Connection::handleReadRequest(const Wt::AsioWrapper::error_code& e,
				   std::size_t bytes_transferred)
{
  LOG_DEBUG(native() << ": handleReadRequest(): " << e.message());

  cancelReadTimer();

  if (!e) {
    rcv_remaining_ = rcv_buffers_.back().data();
    rcv_buffer_size_ = bytes_transferred;
    handleReadRequest0();
  } else if (e != asio::error::operation_aborted &&
	     e != asio::error::bad_descriptor) {
    handleError(e);
  }
}

void Connection::close()
{
  cancelReadTimer();
  cancelWriteTimer();

  LOG_DEBUG(native() << ": close()");

  ConnectionManager_.stop(shared_from_this());
}

bool Connection::closed() const
{
  Connection *self = const_cast<Connection *>(this);
  return !self->socket().is_open();
}

void Connection::handleError(const Wt::AsioWrapper::error_code& e)
{
  LOG_DEBUG(native() << ": error: " << e.message());

  close();
}

void Connection::handleReadBody(ReplyPtr reply)
{
  if (request_.type != Request::WebSocket) {
    /*
     * For a WebSocket: reading and writing may happen in parallel,
     * And writing and reading is asynchronous (post() from within
     * WtReply::consumeWebSocketMessage()
     */
    haveResponse_ = false;
    waitingResponse_ = true;
  }

  RequestParser::ParseResult result = request_parser_
    .parseBody(request_, reply, rcv_remaining_,
	       rcv_buffers_.back().data() + rcv_buffer_size_);

  if (request_.type != Request::WebSocket)
    waitingResponse_ = false;

  if (result == RequestParser::ReadMore) {
    readMore(reply, BODY_TIMEOUT);
  } else if (result == RequestParser::Done && haveResponse_)
    startWriteResponse(reply);
}

void Connection::readMore(ReplyPtr reply, int timeout)
{
  if (!rcv_body_buffer_) {
    rcv_body_buffer_ = true;
    rcv_buffers_.push_back(Buffer());
  }
  startAsyncReadBody(reply, rcv_buffers_.back(), timeout);
}

bool Connection::readAvailable()
{
  try {
    return (rcv_remaining_ < rcv_buffers_.back().data() + rcv_buffer_size_)
      || socket().available();
  } catch (Wt::AsioWrapper::system_error& e) {
    return false; // socket(): bad file descriptor
  }
}

void Connection::detectDisconnect(ReplyPtr reply,
				  const std::function<void()>& callback)
{
  server_->service()
    .post(strand_.wrap(std::bind(&Connection::asyncDetectDisconnect, this, reply, callback)));
}

void Connection::asyncDetectDisconnect(ReplyPtr reply,
				       const std::function<void()>& callback)
{
  if (disconnectCallback_)
    return; // We're already detecting the disconnect

  disconnectCallback_ = callback;

  /*
   * We do not actually expect to receive anything, and if we do, we'll close
   * anyway (see below).
   */
  startAsyncReadBody(reply, rcv_buffers_.back(), 0);
}

void Connection::handleReadBody0(ReplyPtr reply,
                                 const Wt::AsioWrapper::error_code& e,
				 std::size_t bytes_transferred)
{
  LOG_DEBUG(native() << ": handleReadBody0(): " << e.message());

  if (disconnectCallback_) {
    if (e && e != asio::error::operation_aborted) {
      boost::function<void()> f = disconnectCallback_;
      disconnectCallback_ = boost::function<void()>();
      f();
    } else if (!e) {
      LOG_ERROR(native()
		<< ": handleReadBody(): while waiting for disconnect, "
		"received unexpected data, closing");
      close();
    }

    return;
  }

  cancelReadTimer();

  if (!e) {
    rcv_remaining_ = rcv_buffers_.back().data();
    rcv_buffer_size_ = bytes_transferred;
    handleReadBody(reply);
  } else if (e != asio::error::operation_aborted
	     && e != asio::error::bad_descriptor) {
    reply->consumeData(rcv_remaining_, rcv_remaining_, Request::Error);
    handleError(e);
  }
}

void Connection::startWriteResponse(ReplyPtr reply)
{
  haveResponse_ = false;

  if (disconnectCallback_)
    socket().cancel();

  if (state_ & Writing) {
    LOG_ERROR("Connection::startWriteResponse(): connection already writing");
    close();
    server_->service()
      .post(strand_.wrap(std::bind(&Reply::writeDone, reply, false)));
    return;
  }

  std::vector<asio::const_buffer> buffers;
  responseDone_ = reply->nextBuffers(buffers);

  unsigned s = 0;
#ifdef DEBUG
  for (unsigned i = 0; i < buffers.size(); ++i) {
    int size = asio::buffer_size(buffers[i]);
    s += size;
#ifdef DEBUG_DUMP
    char *data = (char *)asio::detail::buffer_cast_helper(buffers[i]);
    for (int j = 0; j < size; ++j)
      std::cerr << data[j];
#endif
  }
#endif

  LOG_DEBUG(native() << " sending: " << s << "(buffers: "
	    << buffers.size() << ")");

  if (!buffers.empty()) {
    startAsyncWriteResponse(reply, buffers, BODY_TIMEOUT);
  } else {
    cancelWriteTimer();
    handleWriteResponse(reply);
  }
}

void Connection::handleWriteResponse(ReplyPtr reply)
{
  LOG_DEBUG(native() << ": handleWriteResponse() " <<
	    haveResponse_ << " " << responseDone_);
  if (haveResponse_)
    startWriteResponse(reply);
  else {
    if (!responseDone_) {
      /*
       * Keep reply open and wait for more data.
       */
    } else {
      reply->logReply(request_handler_.logger());

      if (reply->closeConnection())
	ConnectionManager_.stop(shared_from_this());
      else {
	request_parser_.reset();
	request_.reset();
	responseDone_ = false;

	while (rcv_buffers_.size() > 1)
	  rcv_buffers_.pop_front();

	if (rcv_remaining_ < rcv_buffers_.back().data() + rcv_buffer_size_)
	  handleReadRequest0();
	else
	  startAsyncReadRequest(rcv_buffers_.back(), KEEPALIVE_TIMEOUT);
      }
    }
  }
}

void Connection::handleWriteResponse0(ReplyPtr reply,
                                      const Wt::AsioWrapper::error_code& e,
				      std::size_t bytes_transferred)
{
  LOG_DEBUG(native() << ": handleWriteResponse0(): "
	    << bytes_transferred << " ; " << e.message());

  cancelWriteTimer();

  haveResponse_ = false;
  waitingResponse_ = true;
  reply->writeDone(!e);
  waitingResponse_ = false;

  if (!e) {
    handleWriteResponse(reply);
  } else {
    if (e != asio::error::operation_aborted)
      handleError(e);
  }
}

} // namespace server
} // namespace http
