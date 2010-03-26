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
  finishReply();
  try {
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    socket_.close();
  } catch (asio_system_error&) {
  }
}

typedef void (Connection::*HandleRead)(const asio_error_code&, std::size_t);
typedef void (Connection::*HandleWrite)(const asio_error_code&);

void TcpConnection::startAsyncReadRequest(Buffer& buffer, int timeout)
{
  setTimeout(timeout);

  socket_.async_read_some(asio::buffer(buffer),
      boost::bind(static_cast<HandleRead>(&Connection::handleReadRequest),
		  shared_from_this(),
		  asio::placeholders::error,
		  asio::placeholders::bytes_transferred));
}

void TcpConnection::startAsyncReadBody(Buffer& buffer, int timeout)
{
  setTimeout(timeout);

  socket_.async_read_some(asio::buffer(buffer),
       boost::bind(static_cast<HandleRead>(&Connection::handleReadBody),
		   shared_from_this(),
		   asio::placeholders::error,
		   asio::placeholders::bytes_transferred));
}

void TcpConnection::startAsyncWriteResponse
    (const std::vector<asio::const_buffer>& buffers,
     int timeout)
{
  setTimeout(timeout);

  asio::async_write(socket_, buffers,
       boost::bind(static_cast<HandleWrite>(&Connection::handleWriteResponse),
		   shared_from_this(),
		   asio::placeholders::error));
}

} // namespace server
} // namespace http
