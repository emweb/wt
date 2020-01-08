// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
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

#include <string>

#include "TcpConnection.h"

#ifdef HTTP_WITH_SSL
#include "SslConnection.h"
#endif // HTTP_WITH_SSL

#include "Configuration.h"
#include "ConnectionManager.h"
#include "RequestHandler.h"

#include "Wt/WLogger.h"

namespace http {
namespace server {

class Configuration;

/// The top-level class of the HTTP server.
class Server
{
public:
  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit Server(const Configuration& config, Wt::WServer& wtServer);

  Server(const Server& other) = delete;

  ~Server();

  /// Start the server (called from constructor)
  void start();

  /// Stop the server.
  void stop();

  /// Assumes accept sockets have been closed and reopens them.
  void resume();

  /// Returns the http port number.
  /// If the server listens on multiple port, only the first port is returned
  int httpPort() const;

  Wt::WebController *controller();

  const Configuration &configuration() { return config_; }

  asio::io_service &service();

  SessionProcessManager *sessionManager() { return sessionManager_; }

private:
  std::vector<asio::ip::address> resolveAddress(asio::ip::tcp::resolver &resolver,
                                                const std::string &address);

  struct TcpListener {
    TcpListener(asio::ip::tcp::acceptor &&acceptor,
                TcpConnectionPtr new_connection);

    asio::ip::tcp::acceptor acceptor;
    TcpConnectionPtr new_connection;
  };

  /// Add new TCP listener, called from start()
  void addTcpListener(asio::ip::tcp::resolver &resolver,
                      const std::string &address,
                      const std::string &port);

  /// Add new TCP endpoint, called from addTcpListener
  void addTcpEndpoint(const asio::ip::tcp::endpoint &endpoint,
                      const std::string &address,
                      Wt::AsioWrapper::error_code &errc);

  /// Starts accepting http/https connections
  void startAccept();

  /// Start to connect to a listening TCP socket of the parent
  /// Used for dedicated processes.
  void startConnect(const std::shared_ptr<asio::ip::tcp::socket>& socket);

  /// Connected to the parent, sends the listening port back
  void handleConnected(const std::shared_ptr<asio::ip::tcp::socket>& socket,
                       const Wt::AsioWrapper::error_code& e);

  /// The port has been sent to the parent, close the socket.
  void handlePortSent(const std::shared_ptr<asio::ip::tcp::socket>& socket,
                      const Wt::AsioWrapper::error_code& e,
		      const std::shared_ptr<std::string>& /* buf */);

  /// Handle completion of an asynchronous accept operation.
  void handleTcpAccept(TcpListener *listener, const Wt::AsioWrapper::error_code& e);

  /// Handle a request to stop the server.
  void handleStop();

  /// Handle a request to resume the server.
  void handleResume();

  /// Expire sessions periodically for dedicated processes
  void expireSessions(Wt::AsioWrapper::error_code ec);

  /// The server's configuration
  Configuration config_;

  /// The Wt app server
  Wt::WServer& wt_;

  /// The logger
  Wt::WLogger accessLogger_;

  /// The strand for handleTcpAccept(), handleSslAccept() and handleStop()
  Wt::AsioWrapper::strand accept_strand_;

  /// Acceptors used to listen for incoming http connections.
  std::vector<TcpListener> tcp_listeners_;

#ifdef HTTP_WITH_SSL
  struct SslListener {
    SslListener(asio::ip::tcp::acceptor &&acceptor,
                SslConnectionPtr new_connection);

    asio::ip::tcp::acceptor acceptor;
    SslConnectionPtr new_connection;
  };

  /// Ssl context information
  asio::ssl::context ssl_context_;

  /// Acceptors used to listen for incoming https connections
  std::vector<SslListener> ssl_listeners_;

  /// Add new SSL listener, called from start()
  void addSslListener(asio::ip::tcp::resolver &resolver,
                      const std::string &address,
                      const std::string &port);

  /// Add new SSL endpoint, called from addSslListener
  void addSslEndpoint(const asio::ip::tcp::endpoint &endpoint,
                      const std::string &address,
                      Wt::AsioWrapper::error_code &errc);

  /// Handle completion of an asynchronous SSL accept operation.
  void handleSslAccept(SslListener *listener, const Wt::AsioWrapper::error_code& e);
#endif // HTTP_WITH_SSL

  void handleTimeout(asio::steady_timer *timer,
                     const std::function<void ()>& function,
                     const Wt::AsioWrapper::error_code& err);

  /// The connection manager which owns all live connections.
  ConnectionManager connection_manager_;

  /// Session process manager for DedicatedProcess option
  SessionProcessManager *sessionManager_;

  /// The handler for all incoming requests.
  RequestHandler request_handler_;

  /// For dedicated process deployment: timer to periodically
  /// call WebController::expireSessions()
  asio::steady_timer expireSessionsTimer_;
};

} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP
