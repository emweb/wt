// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_TCP_CONNECTION_HPP
#define HTTP_TCP_CONNECTION_HPP

#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Connection.h"

namespace http {
namespace server {

class RequestHandler;
class Server;

/// Represents a single connection from a client.
class TcpConnection final : public Connection
{
public:
  /// Construct a connection with the given io_service.
  explicit TcpConnection(asio::io_service& io_service, Server *server,
      ConnectionManager& manager, RequestHandler& handler);

  /// Get the socket associated with the connection.
  virtual asio::ip::tcp::socket& socket() override;

  virtual const char *urlScheme() override { return "http"; }

protected:
  virtual void startAsyncReadRequest(Buffer& buffer, int timeout) override;
  virtual void startAsyncReadBody(ReplyPtr reply, Buffer& buffer, int timeout) override;
  virtual void startAsyncWriteResponse
      (ReplyPtr reply, const std::vector<asio::const_buffer>& buffers,
       int timeout) override;

  virtual void stop() override;

  /// Socket for the connection.
  asio::ip::tcp::socket socket_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_HPP
