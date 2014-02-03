// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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

#ifndef HTTP_SSL_CONNECTION_HPP
#define HTTP_SSL_CONNECTION_HPP

#ifdef HTTP_WITH_SSL

#include <boost/asio/ssl.hpp>

#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Connection.h"

namespace http {
namespace server {

class RequestHandler;
class Server;

typedef asio::ssl::stream<asio::ip::tcp::socket> ssl_socket;

/// Represents a single connection from a client.
class SslConnection : public Connection
{
public:
  /// Construct a connection with the given io_service.
  explicit SslConnection(asio::io_service& io_service, Server *server,
      asio::ssl::context& context,
      ConnectionManager& manager, RequestHandler& handler);

  /// Get the socket associated with the connection.
  virtual asio::ip::tcp::socket& socket();

  virtual void start();
  virtual const char *urlScheme() { return "https"; }

protected:

  virtual void stop();

  virtual void startAsyncReadRequest(Buffer& buffer, int timeout);
  virtual void startAsyncReadBody(ReplyPtr reply, Buffer& buffer, int timeout);
  virtual void startAsyncWriteResponse
      (ReplyPtr reply, const std::vector<asio::const_buffer>& buffers,
       int timeout);

private:
  void handleReadRequestSsl(const asio_error_code& e,
                            std::size_t bytes_transferred);
  void handleReadBodySsl(ReplyPtr reply, const asio_error_code& e,
                         std::size_t bytes_transferred);
  void handleHandshake(const asio_error_code& error);
  void stopNextLayer(const boost::system::error_code& ec);

  /// Socket for the connection.
  ssl_socket socket_;

  // SSL shutdown takes many seconds sometimes. Put a limit on it.
  asio::deadline_timer sslShutdownTimer_;
};

typedef boost::shared_ptr<SslConnection> SslConnectionPtr;

} // namespace server
} // namespace http

#endif // HTTP_WITH_SSL

#endif // HTTP_SSL_CONNECTION_HPP
