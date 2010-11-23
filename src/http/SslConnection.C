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

#ifdef HTTP_WITH_SSL

#include <vector>
#include <boost/bind.hpp>

#include "SslConnection.h"
#include "ConnectionManager.h"
#include "Server.h"

namespace http {
namespace server {

SslConnection::SslConnection(asio::io_service& io_service, Server *server,
    asio::ssl::context& context,
    ConnectionManager& manager, RequestHandler& handler)
  : Connection(io_service, server, manager, handler),
    socket_(io_service, context)
{ }

asio::ip::tcp::socket& SslConnection::socket()
{
  return socket_.next_layer();
}

void SslConnection::start()
{
  socket_.async_handshake(asio::ssl::stream_base::server,
      boost::bind(&SslConnection::handleHandshake, this,
		  asio::placeholders::error));
}

void SslConnection::handleHandshake(const asio_error_code& error)
{
  if (!error)
    Connection::start();
  else
    ConnectionManager_.stop(shared_from_this());
}

void SslConnection::stop()
{
  finishReply();
  try {
    socket().close();
  } catch (asio_system_error&) {
  }
}

typedef void (Connection::*HandleRead)(const asio_error_code&, std::size_t);
typedef void (Connection::*HandleWrite)(const asio_error_code&);

void SslConnection::startAsyncReadRequest(Buffer& buffer, int timeout)
{
  setTimeout(timeout);

  socket_.async_read_some(asio::buffer(buffer),
       boost::bind(static_cast<HandleRead>(
         &SslConnection::handleReadRequestSsl),
         shared_from_this(),
         asio::placeholders::error,
         asio::placeholders::bytes_transferred));
}

void SslConnection::handleReadRequestSsl(const asio_error_code& e,
                                         std::size_t bytes_transferred)
{
  // Asio SSL does not perform a write until the read handler
  // has returned. In the normal case, a read handler does not
  // return in case of a recursive event loop, so the SSL write
  // deadlocks a session. Hence, post the processing of the data
  // read, so that the read handler can return here immediately.
  server()->service().post(boost::bind(&Connection::handleReadRequest,
                                       shared_from_this(),
                                       e, bytes_transferred));
}

void SslConnection::startAsyncReadBody(Buffer& buffer, int timeout)
{
  setTimeout(timeout);

  socket_.async_read_some(asio::buffer(buffer),
       boost::bind(static_cast<HandleRead>(&SslConnection::handleReadBodySsl),
		   shared_from_this(),
		   asio::placeholders::error,
		   asio::placeholders::bytes_transferred));
}

void SslConnection::handleReadBodySsl(const asio_error_code& e,
                                      std::size_t bytes_transferred)
{
  // See handleReadRequestSsl for explanation
  server()->service().post(boost::bind(&SslConnection::handleReadBody,
                                       shared_from_this(),
                                       e, bytes_transferred));
}

void SslConnection::startAsyncWriteResponse
    (const std::vector<asio::const_buffer>& buffers, int timeout)
{
  setTimeout(timeout);

  asio::async_write(socket_, buffers,
	boost::bind(static_cast<HandleWrite>(&Connection::handleWriteResponse),
		    shared_from_this(),
		    asio::placeholders::error));
}

} // namespace server
} // namespace http

#endif // HTTP_WITH_SSL
