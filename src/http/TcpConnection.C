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

#include "TcpConnection.h"
#include "Wt/WLogger"

namespace Wt {
  LOGGER("wthttp/async");
}

namespace http {
namespace server {

TcpConnection::TcpConnection(asio::io_service& io_service, Server *server,
    ConnectionManager& manager, RequestHandler& handler)
  : Connection(io_service, server, manager, handler),
    socket_(io_service)
{ }

asio::ip::tcp::socket& TcpConnection::socket()
{
  return socket_;
}

void TcpConnection::stop()
{
  LOG_DEBUG(socket().native() << ": stop()");

  finishReply();

  try {
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    LOG_DEBUG(socket().native() << ": closing socket");
    socket_.close();
  } catch (asio_system_error& e) {
    LOG_DEBUG(socket().native() << ": error " << e.what());
  }

  Connection::stop();
}

void TcpConnection::startAsyncReadRequest(Buffer& buffer, int timeout)
{
  LOG_DEBUG(socket().native() << ": startAsyncReadRequest");

  if (state_ & Reading) {
    stop();
    return;
  }

  setReadTimeout(timeout);

  boost::shared_ptr<TcpConnection> sft 
    = boost::dynamic_pointer_cast<TcpConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (boost::bind(&TcpConnection::handleReadRequest,
				       sft,
				       asio::placeholders::error,
				       asio::placeholders::bytes_transferred)));
}

void TcpConnection::startAsyncReadBody(ReplyPtr reply,
				       Buffer& buffer, int timeout)
{
  LOG_DEBUG(socket().native() << ": startAsyncReadBody");

  if (state_ & Reading) {
    LOG_DEBUG(socket().native() << ": state_ = " << state_);
    stop();
    return;
  }

  setReadTimeout(timeout);

  boost::shared_ptr<TcpConnection> sft 
    = boost::dynamic_pointer_cast<TcpConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (boost::bind(&TcpConnection::handleReadBody,
				       sft,
				       reply,
				       asio::placeholders::error,
				       asio::placeholders::bytes_transferred)));
}

void TcpConnection::startAsyncWriteResponse
     (ReplyPtr reply,
      const std::vector<asio::const_buffer>& buffers,
      int timeout)
{
  LOG_DEBUG(socket().native() << ": startAsyncWriteResponse");

  if (state_ & Writing) {
    LOG_DEBUG(socket().native() << ": state_ = " << state_);
    stop();
    return;
  }

  setWriteTimeout(timeout);

  boost::shared_ptr<TcpConnection> sft 
    = boost::dynamic_pointer_cast<TcpConnection>(shared_from_this());
  asio::async_write(socket_, buffers,
		    strand_.wrap
		    (boost::bind(&TcpConnection::handleWriteResponse,
				 sft,
				 reply,
				 asio::placeholders::error,
				 asio::placeholders::bytes_transferred)));
}

} // namespace server
} // namespace http
