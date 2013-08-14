/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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
#include <boost/bind.hpp>

#include "Connection.h"
#include "ConnectionManager.h"
#include "RequestHandler.h"
#include "StockReply.h"
#include "Server.h"
#include "WebController.h"

/*
 * We need a re-design:
 *   Connection has a request parser and a read-callback
 *   After each read operation:
 *    - request parser is used to determine state
 *    - read-callback is cleared, and called with state.
 *      this may update read-callback and write callback
 *    - when a read or write callback is set:
 *      start write operation -> request parser -> write callback
 *      start write operation -> write callback
 *      these may update read-callback and write callback
 *    - when not reading or writing: we may finish the request or wait for more
 *      new events may be posted out of the blue by setting a callback (and data)
 *   Most important change: callbacks are set to the Connection, and the
 *   request parser is no longer in charge.
 */

namespace Wt {
  LOGGER("wthttp/async");
}

namespace http {
namespace server {

static const int CONNECTION_TIMEOUT = 120; // 2 minutes
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
    server_(server)
{ }

Connection::~Connection()
{
  LOG_DEBUG("~Connection");
}

void Connection::finishReply()
{ 
  if (!request_.uri.empty()) {
    LOG_DEBUG("last request: " << request_.method << " " << request_.uri
	      << " (ws:" << request_.webSocketVersion << ")");
  }
}

void Connection::scheduleStop()
{
  server_->service()
    .post(strand_.wrap(boost::bind(&Connection::stop, shared_from_this())));
}

void Connection::start()
{
  LOG_DEBUG(socket().native() << ": start()");

  request_parser_.reset();
  request_.reset();
  try {
    request_.remoteIP = socket().remote_endpoint().address().to_string();
  } catch (std::exception& e) {
    LOG_ERROR("remote_endpoint() threw: " << e.what());
  }

  asio_error_code ignored_ec;
  socket().set_option(asio::ip::tcp::no_delay(true), ignored_ec);

  startAsyncReadRequest(buffer_, CONNECTION_TIMEOUT);
}

void Connection::setReadTimeout(int seconds)
{
  LOG_DEBUG(socket().native() << " setting read timeout (ws: "
	    << request_.webSocketVersion << ")");
  if (request_.webSocketVersion <= 0)
    state_ = Reading;

  readTimer_.expires_from_now(boost::posix_time::seconds(seconds));
  readTimer_.async_wait(strand_.wrap
			(boost::bind(&Connection::timeout, shared_from_this(),
				     asio::placeholders::error)));
}

void Connection::setWriteTimeout(int seconds)
{
  LOG_DEBUG(socket().native() << " setting write timeout (ws: "
	    << request_.webSocketVersion << ")");
  if (request_.webSocketVersion <= 0)
    state_ = Writing;

  writeTimer_.expires_from_now(boost::posix_time::seconds(seconds));
  writeTimer_.async_wait(strand_.wrap
			 (boost::bind(&Connection::timeout, shared_from_this(),
				      asio::placeholders::error)));
}

void Connection::cancelReadTimer()
{
  LOG_DEBUG(socket().native() << " cancel read timeout");
  state_ = Idle;

  readTimer_.cancel();
}

void Connection::cancelWriteTimer()
{
  LOG_DEBUG(socket().native() << " cancel write timeout");
  state_ = Idle;

  writeTimer_.cancel();
}

void Connection::timeout(const asio_error_code& e)
{
  if (e != asio::error::operation_aborted) {
    asio_error_code ignored_ec;
    socket().shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
    readTimer_.cancel();
    writeTimer_.cancel();
  }
}

void Connection::handleReadRequest0()
{
#ifdef DEBUG
  try {
    LOG_DEBUG(socket().native() << "incoming request: "
	      << socket().remote_endpoint().port() << ": "
	      << std::string(remaining_,
			     std::min((unsigned long)(buffer_.data()
				      - remaining_ + buffer_size_),
				      (long unsigned)1000)));
  } catch (...) {
  }
#endif // DEBUG

  boost::tribool result;
  boost::tie(result, remaining_)
    = request_parser_.parse(request_,
			    remaining_, buffer_.data() + buffer_size_);

  if (result) {
    Reply::status_type status = request_parser_.validate(request_);
    bool doWebSockets = server_->controller()->configuration().webSockets();

    if (doWebSockets)
      request_.enableWebSocket();

    LOG_DEBUG(socket().native() << "request: " << status);

    if (status >= 300)
      sendStockReply(status);
    else {
      if (request_.webSocketVersion >= 0)
	request_.urlScheme = "ws" + urlScheme().substr(4);
      else
	request_.urlScheme = urlScheme();

      try {
	request_.port = socket().local_endpoint().port();
	reply_ = request_handler_.handleRequest(request_);
	reply_->setConnection(shared_from_this());
	moreDataToSendNow_ = true;
      } catch (asio_system_error& e) {
	LOG_ERROR("Error in handleRequest0(): " << e.what());
	handleError(e.code());
	return;
      }

      handleReadBody();
    }
  } else if (!result) {
    sendStockReply(StockReply::bad_request);
  } else {
    startAsyncReadRequest(buffer_, 
			  request_parser_.initialState()
			  ? KEEPALIVE_TIMEOUT 
			  : CONNECTION_TIMEOUT);
  }
}

void Connection::sendStockReply(StockReply::status_type status)
{
  reply_.reset(new StockReply(request_, status, "", server_->configuration()));

  reply_->setConnection(shared_from_this());
  reply_->setCloseConnection();
  moreDataToSendNow_ = true;

  startWriteResponse();
}

void Connection::handleReadRequest(const asio_error_code& e,
				   std::size_t bytes_transferred)
{
  LOG_DEBUG(socket().native() << ": handleReadRequest(): " << e.message());

  cancelReadTimer();

  if (!e) {
    remaining_ = buffer_.data();
    buffer_size_ = bytes_transferred;
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

  LOG_DEBUG(socket().native() << ": close()");

  if (reply_)
    reply_.reset();

  ConnectionManager_.stop(shared_from_this());
}

void Connection::handleError(const asio_error_code& e)
{
  LOG_DEBUG(socket().native() << ": error: " << e.message());

  close();
}

void Connection::handleReadBody()
{
  if (reply_) {
    bool result = request_parser_
      .parseBody(request_, reply_, remaining_, buffer_.data() + buffer_size_);

    if (!result)
      startAsyncReadBody(buffer_, CONNECTION_TIMEOUT);
  } else {
    LOG_DEBUG(socket().native() << "handleReadBody(): no reply");
  }
}

bool Connection::readAvailable()
{
  try {
    return (remaining_ < buffer_.data() + buffer_size_) || socket().available();
  } catch (asio_system_error& e) {
    return false; // socket(): bad file descriptor
  }
}

void Connection::handleReadBody(const asio_error_code& e,
				std::size_t bytes_transferred)
{
  LOG_DEBUG(socket().native() << ": handleReadBody(): " << e.message());

  cancelReadTimer();

  if (!e) {
    remaining_ = buffer_.data();
    buffer_size_ = bytes_transferred;
    handleReadBody();
  } else if (e != asio::error::operation_aborted
	     && e != asio::error::bad_descriptor) {
    if (reply_)
      reply_->consumeData(remaining_, remaining_, Request::Error);

    handleError(e);
  }
}

void Connection::startWriteResponse()
{
  if (state_ != Idle || !reply_) {
    close();
    return;
  }

  std::vector<asio::const_buffer> buffers;
  moreDataToSendNow_ = !reply_->nextBuffers(buffers);

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
  LOG_DEBUG(socket().native() << " sending: " << s << "(buffers: "
	    << buffers.size() << ")");

  if (!buffers.empty()) {
    startAsyncWriteResponse(buffers, CONNECTION_TIMEOUT);
  } else {
    cancelWriteTimer();
    handleWriteResponse();
  }
}

void Connection::handleWriteResponse()
{
  LOG_DEBUG(socket().native() << ": handleWriteResponse() " <<
	    moreDataToSendNow_ << " " << reply_->waitMoreData());
  if (moreDataToSendNow_) {
    startWriteResponse();
  } else {
    if (reply_->waitMoreData()) {
      /*
       * Keep connection open and wait for more data.
       */
    } else {
      reply_->logReply(request_handler_.logger());

      if (reply_->closeConnection()) {
	ConnectionManager_.stop(shared_from_this());
      } else {
	request_parser_.reset();
	request_.reset();
	reply_.reset();

	server_->service()
	  .post(boost::bind(&Connection::handleReadRequest0,
			    shared_from_this()));
      }
    }
  }
}

void Connection::handleWriteResponse(const asio_error_code& e,
    std::size_t bytes_transferred)
{
  LOG_DEBUG(socket().native() << ": handleWriteResponse(): "
      << bytes_transferred << " ; " << e.message());

  cancelWriteTimer();

  if (!e)
    handleWriteResponse();
  else if (e != asio::error::operation_aborted)
    handleError(e);
}

} // namespace server
} // namespace http
