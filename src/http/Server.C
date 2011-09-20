/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "Server.h"
#include "Configuration.h"
#include "WebController.h"

#include <boost/asio/buffer.hpp>

#include <boost/bind.hpp>

#ifdef HTTP_WITH_SSL

#include <boost/asio/ssl.hpp>

#endif // HTTP_WITH_SSL

namespace http {
namespace server {

Server::Server(const Configuration& config, const Wt::Configuration& wtConfig,
               Wt::WebController& controller)
  : config_(config),
    io_service_(),
    accept_strand_(io_service_),
    // post_strand_(io_service_),
    tcp_acceptor_(io_service_),
#ifdef HTTP_WITH_SSL
    ssl_context_(io_service_, asio::ssl::context::sslv23),
    ssl_acceptor_(io_service_),
#endif // HTTP_WITH_SSL
    connection_manager_(),
    request_handler_(config, wtConfig.entryPoints(),
		     accessLogger_),
    controller_(&controller)
{
  if (config.accessLog().empty())
    accessLogger_.setStream(std::cout);
  else {
    accessLogger_.setFile(config.accessLog());
  }

  accessLogger_.addField("remotehost", false);
  accessLogger_.addField("rfc931", false);
  accessLogger_.addField("authuser", false);
  accessLogger_.addField("date", false);
  accessLogger_.addField("request", true);
  accessLogger_.addField("status", false);
  accessLogger_.addField("bytes", false);

  start();
}

void Server::start()
{
  asio::ip::tcp::resolver resolver(io_service_);

  // HTTP
  if (!config_.httpAddress().empty()) {
    std::string httpPort = config_.httpPort();

    asio::ip::tcp::endpoint tcp_endpoint;

    if (httpPort == "0")
      tcp_endpoint.address(asio::ip::address::from_string
			   (config_.httpAddress()));
    else {
#ifndef NO_RESOLVE_ACCEPT_ADDRESS
      asio::ip::tcp::resolver::query tcp_query(config_.httpAddress(),
					       config_.httpPort());
      tcp_endpoint = *resolver.resolve(tcp_query);
#else // !NO_RESOLVE_ACCEPT_ADDRESS
      tcp_endpoint.address
	(asio::ip::address::from_string(config_.httpAddress()));
      tcp_endpoint.port(atoi(httpPort.c_str()));
#endif // NO_RESOLVE_ACCEPT_ADDRESS
    }

    tcp_acceptor_.open(tcp_endpoint.protocol());
    tcp_acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    tcp_acceptor_.bind(tcp_endpoint);
    tcp_acceptor_.listen();

    config_.log("notice") << "Started server: http://"
			  << config_.httpAddress() << ":"
			  << this->httpPort();

    new_tcpconnection_.reset
      (new TcpConnection(io_service_, this, connection_manager_,
			 request_handler_));
  }

  // HTTPS
  if (!config_.httpsAddress().empty()) {
#ifdef HTTP_WITH_SSL
    config_.log("notice")
      << "Starting server: https://" << config_.httpsAddress() << ":"
      << config_.httpsPort();

    ssl_context_.set_options(asio::ssl::context::default_workarounds
			     | asio::ssl::context::no_sslv2
			     | asio::ssl::context::single_dh_use);
    ssl_context_.use_certificate_chain_file(config_.sslCertificateChainFile());
    ssl_context_.use_private_key_file(config_.sslPrivateKeyFile(),
				      asio::ssl::context::pem);
    ssl_context_.use_tmp_dh_file(config_.sslTmpDHFile());
    
    asio::ip::tcp::endpoint ssl_endpoint;
#ifndef NO_RESOLVE_ACCEPT_ADDRESS
    asio::ip::tcp::resolver::query ssl_query(config_.httpsAddress(),
					     config_.httpsPort());
    ssl_endpoint = *resolver.resolve(ssl_query);
#else // !NO_RESOLVE_ACCEPT_ADDRESS
    ssl_endpoint.address(asio::ip::address::from_string(config_.httpsAddress()));
    ssl_endpoint.port(atoi(httpsPort.c_str()));
#endif // NO_RESOLVE_ACCEPT_ADDRESS

    ssl_acceptor_.open(ssl_endpoint.protocol());
    ssl_acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    ssl_acceptor_.bind(ssl_endpoint);
    ssl_acceptor_.listen();

    new_sslconnection_.reset
      (new SslConnection(io_service_, this, ssl_context_, connection_manager_,
			 request_handler_));

#else // HTTP_WITH_SSL
    config_.log("error")
      << "Wthttpd was built without support for SSL: cannot start https server.";
#endif // HTTP_WITH_SSL
  }

  // Win32 cancels the non-blocking accept when the thread that called
  // accept exits. To avoid that this happens when called within the
  // WServer context, we post the action of calling accept to one of
  // the threads in the threadpool.
  io_service_.post(boost::bind(&Server::startAccept, this));
}

int Server::httpPort() const
{
  return tcp_acceptor_.local_endpoint().port();
}

void Server::schedule(int milliSeconds,
		      const boost::function<void ()>& function)
{
  /*
   * We would need to serialize events for a single session and thus maintain
   * a queue per sessionId ?
   */
  // io_service_.post(post_strand_.wrap(function));

  if (milliSeconds == 0) {
    io_service_.post(function);
  } else {
    asio::deadline_timer *timer = new asio::deadline_timer(io_service_);
    timer->expires_from_now(boost::posix_time::milliseconds(milliSeconds));
    timer->async_wait(boost::bind(&Server::handleTimeout, this,
				  timer, function, asio::placeholders::error));
  }
}

void Server::handleTimeout(asio::deadline_timer *timer,
			   const boost::function<void ()>& function,
			   const asio_error_code& e)
{
  if (!e)
    function();

  delete timer;
}

void Server::startAccept()
{
  /*
   * For simplicity, we are using the same accept_strand_ for Tcp
   * and Ssl, to prevent the close() from within handleStop() and
   * async_accept() methods to be called simultaneously.
   *
   * While this also prevents simultaneously accepting a new Tcp and
   * Ssl connection, this performance impact is negligible (and both
   * need to access the ConnectionManager mutex in any case).
   */
  if (new_tcpconnection_) {
    tcp_acceptor_.async_accept(new_tcpconnection_->socket(),
			accept_strand_.wrap(
			       boost::bind(&Server::handleTcpAccept, this,
					   asio::placeholders::error)));
  }

#ifdef HTTP_WITH_SSL
  if (new_sslconnection_) {
    ssl_acceptor_.async_accept(new_sslconnection_->socket(),
	                accept_strand_.wrap(
			       boost::bind(&Server::handleSslAccept, this,
					   asio::placeholders::error)));
  }
#endif // HTTP_WITH_SSL
}

Server::~Server()
{ }

void Server::run()
{
  // The io_service::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  io_service_.run();
}

void Server::stop()
{
  // Post a call to the stop function so that server::stop() is safe
  // to call from any thread, and not simultaneously with waiting for
  // a new async_accept() call.
  io_service_.post(accept_strand_.wrap
		   (boost::bind(&Server::handleStop, this)));
}

void Server::resume()
{
  io_service_.post(boost::bind(&Server::handleResume, this));
}

void Server::handleResume()
{
  tcp_acceptor_.close();

#ifdef HTTP_WITH_SSL
  ssl_acceptor_.close();
#endif // HTTP_WITH_SSL
  
  start();
}

void Server::handleTcpAccept(const asio_error_code& e)
{
  if (!e) {
    connection_manager_.start(new_tcpconnection_);
    new_tcpconnection_.reset(new TcpConnection(io_service_, this,
          connection_manager_, request_handler_));
    tcp_acceptor_.async_accept(new_tcpconnection_->socket(),
	                accept_strand_.wrap(
                    boost::bind(&Server::handleTcpAccept, this,
				asio::placeholders::error)));
  }
}

#ifdef HTTP_WITH_SSL
void Server::handleSslAccept(const asio_error_code& e)
{
  if (!e)
  {
    connection_manager_.start(new_sslconnection_);
    new_sslconnection_.reset(new SslConnection(io_service_, this,
          ssl_context_, connection_manager_, request_handler_));
    ssl_acceptor_.async_accept(new_sslconnection_->socket(),
	                accept_strand_.wrap(
	           boost::bind(&Server::handleSslAccept, this,
			       asio::placeholders::error)));
  }
}
#endif // HTTP_WITH_SSL

void Server::handleStop()
{
  // The server is stopped by cancelling all outstanding asynchronous
  // operations. Once all operations have finished the io_service::run() call
  // will exit.
  tcp_acceptor_.close();

#ifdef HTTP_WITH_SSL
  ssl_acceptor_.close();
#endif // HTTP_WITH_SSL

  connection_manager_.stopAll();
}

} // namespace server
} // namespace http
