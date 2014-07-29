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
    server_(server),
    waitingResponse_(false),
    haveResponse_(false),
    responseDone_(false)
{ }

Connection::~Connection()
{
  LOG_DEBUG("~Connection");
}

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
    .post(strand_.wrap(boost::bind(&Connection::stop, shared_from_this())));
}

void Connection::start()
{
  LOG_DEBUG(socket().native() << ": start()");

  request_parser_.reset();
  request_.reset();
  try {
    request_.remoteIP = socket().remote_endpoint().address().to_string();
    request_.port = socket().local_endpoint().port();
  } catch (std::exception& e) {
    LOG_ERROR("remote_endpoint() threw: " << e.what());
  }

  asio_error_code ignored_ec;
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
  LOG_DEBUG(socket().native() << " setting read timeout (ws: "
	    << request_.webSocketVersion << ")");
  state_ |= Reading;

  readTimer_.expires_from_now(boost::posix_time::seconds(seconds));
  readTimer_.async_wait(boost::bind(&Connection::timeout, shared_from_this(),
				    asio::placeholders::error));
}

void Connection::setWriteTimeout(int seconds)
{
  LOG_DEBUG(socket().native() << " setting write timeout (ws: "
	    << request_.webSocketVersion << ")");
  state_ |= Writing;

  writeTimer_.expires_from_now(boost::posix_time::seconds(seconds));
  writeTimer_.async_wait(boost::bind(&Connection::timeout, shared_from_this(),
				      asio::placeholders::error));
}

void Connection::cancelReadTimer()
{
  LOG_DEBUG(socket().native() << " cancel read timeout");
  state_.clear(Reading);

  readTimer_.cancel();
}

void Connection::cancelWriteTimer()
{
  LOG_DEBUG(socket().native() << " cancel write timeout");
  state_.clear(Writing);

  writeTimer_.cancel();
}

void Connection::timeout(const asio_error_code& e)
{
  if (e != asio::error::operation_aborted)
    strand_.post(boost::bind(&Connection::doTimeout, shared_from_this()));
}

void Connection::doTimeout()
{
  asio_error_code ignored_ec;
  socket().shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
  readTimer_.cancel();
  writeTimer_.cancel();
}

void Connection::handleReadRequest0()
{
  Buffer& buffer = rcv_buffers_.back();

#ifdef DEBUG
  try {
    LOG_DEBUG(socket().native() << "incoming request: "
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
			    rcv_remaining_, buffer.data() + rcv_buffer_size_);

  if (result) {
    Reply::status_type status = request_parser_.validate(request_);
    // FIXME: Let the reply decide whether we're doing websockets, move this logic to WtReply
    bool doWebSockets = server_->controller()->configuration().webSockets() &&
			(server_->controller()->configuration().sessionPolicy() != Wt::Configuration::DedicatedProcess ||
			 server_->configuration().parentPort() != -1);

    if (doWebSockets)
      request_.enableWebSocket();

    LOG_DEBUG(socket().native() << "request: " << status);

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
      } catch (asio_system_error& e) {
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

void Connection::handleReadRequest(const asio_error_code& e,
				   std::size_t bytes_transferred)
{
  LOG_DEBUG(socket().native() << ": handleReadRequest(): " << e.message());

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

  LOG_DEBUG(socket().native() << ": close()");

  ConnectionManager_.stop(shared_from_this());
}

bool Connection::closed() const
{
  Connection *self = const_cast<Connection *>(this);
  return !self->socket().is_open();
}

void Connection::handleError(const asio_error_code& e)
{
  LOG_DEBUG(socket().native() << ": error: " << e.message());

  close();
}

void Connection::handleReadBody(ReplyPtr reply)
{
  haveResponse_ = false;
  waitingResponse_ = true;

  RequestParser::ParseResult result = request_parser_
    .parseBody(request_, reply, rcv_remaining_,
	       rcv_buffers_.back().data() + rcv_buffer_size_);

  waitingResponse_ = false;

  if (result == RequestParser::ReadMore) {
    readMore(reply);
  } else if (result == RequestParser::Done && haveResponse_)
    startWriteResponse(reply);
}

void Connection::readMore(ReplyPtr reply)
{
  if (!rcv_body_buffer_) {
    rcv_body_buffer_ = true;
    rcv_buffers_.push_back(Buffer());
  }
  startAsyncReadBody(reply, rcv_buffers_.back(), CONNECTION_TIMEOUT);
}

bool Connection::readAvailable()
{
  try {
    return (rcv_remaining_ < rcv_buffers_.back().data() + rcv_buffer_size_)
      || socket().available();
  } catch (asio_system_error& e) {
    return false; // socket(): bad file descriptor
  }
}

void Connection::handleReadBody(ReplyPtr reply,
				const asio_error_code& e,
				std::size_t bytes_transferred)
{
  LOG_DEBUG(socket().native() << ": handleReadBody(): " << e.message());

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

  if (state_ & Writing) {
    LOG_ERROR("Connection::startWriteResponse(): connection already writing");
    close();
    server_->service()
      .post(strand_.wrap(boost::bind(&Reply::writeDone, reply, false)));
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

  LOG_DEBUG(socket().native() << " sending: " << s << "(buffers: "
	    << buffers.size() << ")");

  if (!buffers.empty()) {
    startAsyncWriteResponse(reply, buffers, CONNECTION_TIMEOUT);
  } else {
    cancelWriteTimer();
    handleWriteResponse(reply);
  }
}

void Connection::handleWriteResponse(ReplyPtr reply)
{
  LOG_DEBUG(socket().native() << ": handleWriteResponse() " <<
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

void Connection::handleWriteResponse(ReplyPtr reply,
				     const asio_error_code& e,
				     std::size_t bytes_transferred)
{
  LOG_DEBUG(socket().native() << ": handleWriteResponse(): "
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
