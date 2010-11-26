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

#include <vector>
#include <boost/bind.hpp>

#include "Connection.h"
#include "ConnectionManager.h"
#include "RequestHandler.h"
#include "StockReply.h"

//#define DEBUG

namespace http {
namespace server {

static const int CONNECTION_TIMEOUT = 120; // 2 minutes
static const int KEEPALIVE_TIMEOUT  = 10;  // 10 seconds

Connection::Connection(asio::io_service& io_service, Server *server,
    ConnectionManager& manager, RequestHandler& handler)
  : ConnectionManager_(manager),
    request_handler_(handler),
    timer_(io_service),
    request_parser_(server),
    server_(server)
{ }

Connection::~Connection()
{ }

void Connection::finishReply()
{
  if (reply_.get())
    reply_->setConnection(0);
}

void Connection::start()
{
  request_parser_.reset();
  request_.reset();
  try {
    request_.remoteIP = socket().remote_endpoint().address().to_string();
  } catch (std::exception& e) {
    std::cerr << "remote_endpoint() threw: " << e.what() << std::endl;
  }

  socket().set_option(asio::ip::tcp::no_delay(true));

  startAsyncReadRequest(buffer_, CONNECTION_TIMEOUT);
}

void Connection::setTimeout(int seconds)
{
  timer_.expires_from_now(boost::posix_time::seconds(seconds));
  timer_.async_wait(boost::bind(&Connection::timeout, shared_from_this(),
  				asio::placeholders::error));  
}

void Connection::cancelTimer()
{
  timer_.cancel();
}

void Connection::timeout(const asio_error_code& e)
{
  if (e != asio::error::operation_aborted) {
    asio_error_code ignored_ec;
    socket().shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }
}

void Connection::handleReadRequest0()
{
#ifdef DEBUG
    std::cerr << "Incoming request: "
	    << socket().remote_endpoint().port() << ": "
	      << std::string(remaining_,
			     std::min(buffer_.data()
				      - remaining_ + buffer_size_,
				      (long unsigned)150)) << std::endl;
#endif // DEBUG

  boost::tribool result;
  boost::tie(result, remaining_)
    = request_parser_.parse(request_,
			    remaining_, buffer_.data() + buffer_size_);

  if (result) {
    Reply::status_type status = request_parser_.validate(request_);
    if (status >= 300)
      sendStockReply(status);
    else {
      if (request_.isWebSocketRequest())
	request_.urlScheme = "ws" + urlScheme().substr(4);
      else
	request_.urlScheme = urlScheme();

      request_.port = socket().local_endpoint().port();
      reply_ = request_handler_.handleRequest(request_);
      reply_->setConnection(this);
      moreDataToSend_ = true;

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
  if (reply_)
    reply_->release();
  reply_.reset(new StockReply(request_, status, "",
			      request_handler_.getErrorRoot()));

  reply_->setConnection(this);
  reply_->setCloseConnection();
  moreDataToSend_ = true;

  startWriteResponse();
}

void Connection::handleReadRequest(const asio_error_code& e,
				   std::size_t bytes_transferred)
{
  cancelTimer();

  if (!e) {
    remaining_ = buffer_.data();
    buffer_size_ = bytes_transferred;
    handleReadRequest0();
  } else if (e != asio::error::operation_aborted
	     && e != asio::error::bad_descriptor) {
    handleError(e);
  }
}

void Connection::handleError(const asio_error_code& e)
{
  if (reply_)
    reply_->release();
  // std::cerr << "asio error: " << this << " " << e.message() << std::endl;
  ConnectionManager_.stop(shared_from_this());
}

void Connection::handleReadBody()
{
  bool result = request_parser_
    .parseBody(request_, reply_, remaining_, buffer_.data() + buffer_size_);

  if (!result)
    startAsyncReadBody(buffer_, CONNECTION_TIMEOUT);
}

bool Connection::readAvailable()
{
  return (remaining_ < buffer_.data() + buffer_size_) || socket().available();
}

void Connection::handleReadBody(const asio_error_code& e,
				std::size_t bytes_transferred)
{
  cancelTimer();

  if (!e) {
    remaining_ = buffer_.data();
    buffer_size_ = bytes_transferred;
    handleReadBody();
  } else if (e != asio::error::operation_aborted
	     && e != asio::error::bad_descriptor) {
    handleError(e);
  }
}

void Connection::startWriteResponse()
{
  std::vector<asio::const_buffer> buffers;
  moreDataToSend_ = !reply_->nextBuffers(buffers);

#ifdef DEBUG
  std::cerr << "Sending" << std::endl;
  for (unsigned i = 0; i < buffers.size(); ++i) {
    char *data = (char *)asio::detail::buffer_cast_helper(buffers[i]);
    int size = asio::buffer_size(buffers[i]);

    for (int j = 0; j < size; ++j)
      std::cerr << data[j];
  }
#endif

  if (!buffers.empty()) {
    startAsyncWriteResponse(buffers, CONNECTION_TIMEOUT);
  } else {
    cancelTimer();
    handleWriteResponse();
  }
}

void Connection::handleWriteResponse()
{
  if (moreDataToSend_) {
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
	handleReadRequest0();
      }
    }
  }
}

void Connection::handleWriteResponse(const asio_error_code& e)
{
  cancelTimer();

  if (e != asio::error::operation_aborted) {
    if (e) {
      handleError(e);
    }
    handleWriteResponse();
  }
}

} // namespace server
} // namespace http
