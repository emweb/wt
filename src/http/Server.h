// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#ifdef HTTP_WITH_SSL
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
namespace asio = boost::asio;
#endif // HTTP_WITH_SSL

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/version.hpp>

#include "TcpConnection.h"

#ifdef HTTP_WITH_SSL
#include "SslConnection.h"
#endif // HTTP_WITH_SSL

#include "Configuration.h"
#include "ConnectionManager.h"
#include "RequestHandler.h"

#include "Wt/WLogger"

namespace http {
namespace server {

class Configuration;

/// The top-level class of the HTTP server.
class Server
  : private boost::noncopyable
{
public:
  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit Server(const Configuration& config, Wt::WServer& wtServer);

  ~Server();

  /// Start the server (called from constructor)
  void start();

  /// Stop the server.
  void stop();

  /// Assumes accept sockets have been closed and reopens them.
  void resume();

  /// Returns the http port number.
  int httpPort() const;

  Wt::WebController *controller();

  const Configuration &configuration() { return config_; }

  asio::io_service &service();

private:
  /// Starts accepting http/https connections
  void startAccept();

  /// Handle completion of an asynchronous accept operation.
  void handleTcpAccept(const asio_error_code& e);

  /// Handle a request to stop the server.
  void handleStop();

  /// Handle a request to resume the server.
  void handleResume();

  /// The server's configuration
  Configuration config_;

  /// The Wt app server
  Wt::WServer& wt_;

  /// The logger
  Wt::WLogger accessLogger_;

  /// The strand for handleTcpAccept(), handleSslAccept() and handleStop()
  asio::strand accept_strand_;

  /// The strand for schedule()
  // asio::strand schedule_strand_;

  /// Acceptor used to listen for incoming http connections.
  asio::ip::tcp::acceptor tcp_acceptor_;

#ifdef HTTP_WITH_SSL
  /// Ssl context information
  asio::ssl::context ssl_context_;

  /// Acceptor used to listen for incoming https connections
  asio::ip::tcp::acceptor ssl_acceptor_;

  /// Handle completion of an asynchronous SSL accept operation.
  void handleSslAccept(const asio_error_code& e);

  /// The next SSL connection to be accepted.
  SslConnectionPtr new_sslconnection_;
#endif // HTTP_WITH_SSL

void handleTimeout(asio::deadline_timer *timer,
		   const boost::function<void ()>& function,
		   const asio_error_code& err);

  /// The connection manager which owns all live connections.
  ConnectionManager connection_manager_;

  /// The next TCP connection to be accepted.
  TcpConnectionPtr new_tcpconnection_;

  /// The handler for all incoming requests.
  RequestHandler request_handler_;
};

} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP
