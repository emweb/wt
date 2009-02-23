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

#ifdef BOOST_ASIO
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
namespace asio = boost::asio;
#else // !BOOST_ASIO
#include <asio.hpp>
#include <asio/ssl.hpp>
#endif // BOOST_ASIO

#endif // HTTP_WITH_SSL

#if defined(THREADED) && BOOST_VERSION < 103600

#ifdef BOOST_ASIO
#include <boost/asio/detail/select_reactor.hpp>
#else // !BOOST_ASIO
#include <asio/detail/select_reactor.hpp>
#endif // BOOST_ASIO

#endif // defined(THREADED) && BOOST_VERSION < 103600

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/version.hpp>

#include "TcpConnection.h"

#ifdef HTTP_WITH_SSL
#include "SslConnection.h"
#endif // HTTP_WITH_SSL

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
  explicit Server(const Configuration& config,
		  const Wt::Configuration& wtConfig);

  ~Server();

  /// Run the server's io_service loop.
  void run();

  /// Stop the server.
  void stop();

  /// Select for read on descriptor
  void select_read(int descriptor);

  /// Select for read on descriptor
  void select_write(int descriptor);

  /// Select for exceptions on descriptor
  void select_except(int descriptor);

  /// Stop selecting on the descriptor
  void stop_select(int descriptor);

  static Server* instance() { return instance_; }

private:
  /// Handle completion of an asynchronous accept operation.
  void handleTcpAccept(const asio_error_code& e);

  /// Handle a request to stop the server.
  void handleStop();

  /// The logger
  Wt::WLogger   accessLogger_;

  /// The io_service used to perform asynchronous operations.
  asio::io_service io_service_;

  /// The strand for handleTcpAccept(), handleSslAccept() and handleStop()
  asio::strand accept_strand_;

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

#ifdef THREADED
#if BOOST_VERSION < 103600
  /// Reactor that listens for selects() on auxiliary sockets. 
  asio::detail::select_reactor<false> select_reactor_;
#else
  struct SelectInfo {
    asio::ip::tcp::socket *readSocket, *writeSocket;

    SelectInfo() : readSocket(0), writeSocket(0) { }
  };
  std::map<int, SelectInfo> notifyingSockets_;
  boost::recursive_mutex    notifyingSocketsMutex_;
#endif
#endif

  /// The connection manager which owns all live connections.
  ConnectionManager connection_manager_;

  /// The next TCP connection to be accepted.
  TcpConnectionPtr new_tcpconnection_;

  /// The handler for all incoming requests.
  RequestHandler request_handler_;

  enum SelectOp { Read, Write };
  bool socketSelected(int descriptor, const asio_error_code& e,
		      std::size_t bytes_transferred, SelectOp op);

  static Server *instance_;
};

} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP
