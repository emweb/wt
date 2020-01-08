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

#include <vector>

#include "TcpConnection.h"
#include "Wt/WLogger.h"

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
  LOG_DEBUG(native() << ": stop()");

  finishReply();

  try {
    Wt::AsioWrapper::error_code ignored_ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
    LOG_DEBUG(native() << ": closing socket");
    socket_.close();
  } catch (Wt::AsioWrapper::system_error& e) {
    LOG_DEBUG(native() << ": error " << e.what());
  }

  Connection::stop();
}

void TcpConnection::startAsyncReadRequest(Buffer& buffer, int timeout)
{
  LOG_DEBUG(native() << ": startAsyncReadRequest");

  if (state_ & Reading) {
    LOG_DEBUG(native() << ": state_ = "
	      << (state_ & Reading ? "reading " : "")
	      << (state_ & Writing ? "writing " : ""));
    stop();
    return;
  }

  setReadTimeout(timeout);

  std::shared_ptr<TcpConnection> sft 
    = std::static_pointer_cast<TcpConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (std::bind(&TcpConnection::handleReadRequest,
				     sft,
				     std::placeholders::_1,
				     std::placeholders::_2)));
}

void TcpConnection::startAsyncReadBody(ReplyPtr reply,
				       Buffer& buffer, int timeout)
{
  LOG_DEBUG(native() << ": startAsyncReadBody");

  if (state_ & Reading) {
    LOG_DEBUG(native() << ": state_ = "
	      << (state_ & Reading ? "reading " : "")
	      << (state_ & Writing ? "writing " : ""));
    stop();
    return;
  }

  setReadTimeout(timeout);

  std::shared_ptr<TcpConnection> sft 
    = std::static_pointer_cast<TcpConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (std::bind(&TcpConnection::handleReadBody0,
				     sft,
				     reply,
				     std::placeholders::_1,
				     std::placeholders::_2)));
}

void TcpConnection::startAsyncWriteResponse
     (ReplyPtr reply,
      const std::vector<asio::const_buffer>& buffers,
      int timeout)
{
  LOG_DEBUG(native() << ": startAsyncWriteResponse");

  if (state_ & Writing) {
    LOG_DEBUG(native() << ": state_ = "
	      << (state_ & Reading ? "reading " : "")
	      << (state_ & Writing ? "writing " : ""));
    stop();
    return;
  }

  setWriteTimeout(timeout);

  std::shared_ptr<TcpConnection> sft 
    = std::static_pointer_cast<TcpConnection>(shared_from_this());
  asio::async_write(socket_, buffers,
		    strand_.wrap
		    (std::bind(&TcpConnection::handleWriteResponse0,
			       sft,
			       reply,
			       std::placeholders::_1,
			       std::placeholders::_2)));
}

} // namespace server
} // namespace http
