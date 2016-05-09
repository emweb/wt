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

#include <boost/asio.hpp>

#include <Wt/WIOService>
#include <Wt/WServer>

#include "Server.h"
#include "Configuration.h"
#include "WebController.h"
#include "WebUtils.h"

#include <boost/bind.hpp>

#ifdef HTTP_WITH_SSL

#include <boost/asio/ssl.hpp>

#endif // HTTP_WITH_SSL

namespace {
  std::string bindError(asio::ip::tcp::endpoint ep, 
			boost::system::system_error e) {
    std::stringstream ss;
    ss << "Error occurred when binding to " 
       << ep.address().to_string() 
       << ":"
       << ep.port() 
       << std::endl
       << e.what();
    return ss.str();
  }

#ifdef HTTP_WITH_SSL
  SSL_CTX *nativeContext(asio::ssl::context& context)
  {
#if BOOST_VERSION >= 104700
    return context.native_handle();
#else //BOOST_VERSION < 104700
    return context.impl();
#endif //BOOST_VERSION >= 104700
  }
#endif //HTTP_WITH_SSL

  // The interval to run WebController::expireSessions(),
  // when running as a dedicated process.
  static const int SESSION_EXPIRE_INTERVAL = 5;
}

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

Server::Server(const Configuration& config, Wt::WServer& wtServer)
  : config_(config),
    wt_(wtServer),
    accept_strand_(wt_.ioService()),
    // post_strand_(ioService_),
    tcp_acceptor_(wt_.ioService()),
#ifdef HTTP_WITH_SSL
    ssl_context_(wt_.ioService(), asio::ssl::context::sslv23),
    ssl_acceptor_(wt_.ioService()),
#endif // HTTP_WITH_SSL
    connection_manager_(),
    sessionManager_(0),
    request_handler_(config, wt_.configuration(), accessLogger_),
    expireSessionsTimer_(wt_.ioService())
{
  if (config.parentPort() != -1) {
    accessLogger_.configure(std::string("-*"));
    wtServer.dedicatedProcessEnabled_ = true;
  } else if (config.accessLog().empty())
    accessLogger_.setStream(std::cout);
  else if (config.accessLog() == "-")
    accessLogger_.configure(std::string("-*"));
  else
    accessLogger_.setFile(config.accessLog());

  if (wt_.configuration().sessionPolicy() == Wt::Configuration::DedicatedProcess &&
      config.parentPort() == -1) {
    sessionManager_ = new SessionProcessManager(wt_.ioService(), wt_.configuration());
    request_handler_.setSessionManager(sessionManager_);
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

asio::io_service& Server::service()
{
  return wt_.ioService();
}

Wt::WebController *Server::controller()
{
  return wt_.controller();
}

void Server::start()
{
  if (config_.parentPort() != -1) {
    expireSessionsTimer_.expires_from_now(boost::posix_time::seconds(SESSION_EXPIRE_INTERVAL));
    expireSessionsTimer_.async_wait(boost::bind(&Server::expireSessions, this,
	  asio::placeholders::error));
  }

  asio::ip::tcp::resolver resolver(wt_.ioService());

  // HTTP
  if (!config_.httpAddress().empty() || config_.parentPort() != -1) {
    asio::ip::tcp::endpoint tcp_endpoint;

    if (config_.parentPort() == -1) {
      std::string httpPort = config_.httpPort();

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
    } else {
      tcp_endpoint = asio::ip::tcp::endpoint(
	  asio::ip::address_v4::loopback(), 0);
    }

    tcp_acceptor_.open(tcp_endpoint.protocol());
    tcp_acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    try {
      tcp_acceptor_.bind(tcp_endpoint);
    } catch (boost::system::system_error e) {
      LOG_ERROR_S(&wt_, bindError(tcp_endpoint, e));
      throw;
    }
    tcp_acceptor_.listen();

    LOG_INFO_S(&wt_, "started server: http://" << 
	       config_.httpAddress() << ":" << this->httpPort());

    new_tcpconnection_.reset
      (new TcpConnection(wt_.ioService(), this, connection_manager_,
			 request_handler_));
  }

  // HTTPS
  if (!config_.httpsAddress().empty() && config_.parentPort() == -1) {
#ifdef HTTP_WITH_SSL
    LOG_INFO_S(&wt_, "starting server: https://" <<
	       config_.httpsAddress() << ":" << config_.httpsPort());

    if (config_.hasSslPasswordCallback())
      ssl_context_.set_password_callback(config_.sslPasswordCallback());

    int sslOptions = asio::ssl::context::default_workarounds
      | asio::ssl::context::no_sslv2
      | asio::ssl::context::single_dh_use;

    if (!config_.sslEnableV3())
      sslOptions |= asio::ssl::context::no_sslv3;

    ssl_context_.set_options(sslOptions);

    if (config_.sslClientVerification() == "none") {
      ssl_context_.set_verify_mode(asio::ssl::context::verify_none);
	} else if (config_.sslClientVerification() == "once") {
	  ssl_context_.set_verify_mode(asio::ssl::context::verify_client_once);
      ssl_context_.load_verify_file(config_.sslCaCertificates());
    } else if (config_.sslClientVerification() == "optional") {
      ssl_context_.set_verify_mode(asio::ssl::context::verify_peer);
      ssl_context_.load_verify_file(config_.sslCaCertificates());
    } else {
      // assume 'required'
      ssl_context_.set_verify_mode(asio::ssl::context::verify_peer |
        asio::ssl::context::verify_fail_if_no_peer_cert);
      ssl_context_.load_verify_file(config_.sslCaCertificates());
    }

    ssl_context_.use_certificate_chain_file(config_.sslCertificateChainFile());
    ssl_context_.use_private_key_file(config_.sslPrivateKeyFile(),
				      asio::ssl::context::pem);
    ssl_context_.use_tmp_dh_file(config_.sslTmpDHFile());
    
    SSL_CTX *native_ctx = nativeContext(ssl_context_);
    
    if (config_.sslCipherList().size()) {
      if (!SSL_CTX_set_cipher_list(native_ctx, config_.sslCipherList().c_str())) {
        throw Wt::WServer::Exception(
          "failed to select ciphers for cipher list "
          + config_.sslCipherList());
      }
    }

    std::string sessionId = Wt::WRandom::generateId(SSL_MAX_SSL_SESSION_ID_LENGTH);
    SSL_CTX_set_session_id_context(native_ctx,
      reinterpret_cast<const unsigned char *>(sessionId.c_str()), sessionId.size());

    asio::ip::tcp::endpoint ssl_endpoint;
#ifndef NO_RESOLVE_ACCEPT_ADDRESS
    asio::ip::tcp::resolver::query ssl_query(config_.httpsAddress(),
					     config_.httpsPort());
    ssl_endpoint = *resolver.resolve(ssl_query);
#else // !NO_RESOLVE_ACCEPT_ADDRESS
    ssl_endpoint.address(asio::ip::address::from_string(config_.httpsAddress()));
    ssl_endpoint.port(atoi(config_.httpsPort().c_str()));
#endif // NO_RESOLVE_ACCEPT_ADDRESS

    ssl_acceptor_.open(ssl_endpoint.protocol());
    ssl_acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    try {
      ssl_acceptor_.bind(ssl_endpoint);
    } catch (boost::system::system_error e) {
      LOG_ERROR_S(&wt_, bindError(ssl_endpoint, e);)
      throw;
    }
    ssl_acceptor_.listen();

    new_sslconnection_.reset
      (new SslConnection(wt_.ioService(), this, ssl_context_, connection_manager_,
			 request_handler_));

#else // HTTP_WITH_SSL
    LOG_ERROR_S(&wt_, "built without support for SSL: "
		"cannot start https server.");
#endif // HTTP_WITH_SSL
  }

  // Win32 cancels the non-blocking accept when the thread that called
  // accept exits. To avoid that this happens when called within the
  // WServer context, we post the action of calling accept to one of
  // the threads in the threadpool.
  wt_.ioService().post(boost::bind(&Server::startAccept, this));

  if (config_.parentPort() != -1) {
    // This is a child process, connect to parent to
    // announce the listening port.
    boost::shared_ptr<asio::ip::tcp::socket> parentSocketPtr
      (new asio::ip::tcp::socket(wt_.ioService()));
    wt_.ioService().post(boost::bind(&Server::startConnect, this, parentSocketPtr));
  }
}

int Server::httpPort() const
{
  return tcp_acceptor_.local_endpoint().port();
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

void Server::startConnect(const boost::shared_ptr<asio::ip::tcp::socket>& socket)
{
  socket->async_connect(
      asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), config_.parentPort()),
	boost::bind(&Server::handleConnected, this,
		    socket,
		    asio::placeholders::error));
}

void Server::handleConnected(const boost::shared_ptr<asio::ip::tcp::socket>& socket,
			     const asio_error_code& err)
{
  if (!err) {
    boost::shared_ptr<std::string> buf(new std::string(
      boost::lexical_cast<std::string>(tcp_acceptor_.local_endpoint().port())));
    socket->async_send(asio::buffer(*buf),
      boost::bind(&Server::handlePortSent, this, socket, err, buf));
  } else {
    LOG_ERROR_S(&wt_, "child process couldn't connect to parent to send listening port: " << err.message());
  }
}

void Server::handlePortSent(const boost::shared_ptr<asio::ip::tcp::socket>& socket,
			    const asio_error_code& err, const boost::shared_ptr<std::string>& /*buf*/)
{
  if (err) {
    LOG_ERROR_S(&wt_, "child process couldn't send listening port: " << err.message());
  }
  boost::system::error_code ignored_ec;
  if(socket.get()) {
	socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	socket->close();
  }

}

Server::~Server()
{
  if (sessionManager_)
    delete sessionManager_;
}

void Server::stop()
{
  // Post a call to the stop function so that server::stop() is safe
  // to call from any thread, and not simultaneously with waiting for
  // a new async_accept() call.
  wt_.ioService().post(accept_strand_.wrap
		   (boost::bind(&Server::handleStop, this)));
}

void Server::resume()
{
  wt_.ioService().post(boost::bind(&Server::handleResume, this));
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
    new_tcpconnection_.reset(new TcpConnection(wt_.ioService(), this,
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
    new_sslconnection_.reset(new SslConnection(wt_.ioService(), this,
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
  if (sessionManager_)
    sessionManager_->stop();

  expireSessionsTimer_.cancel();

  // The server is stopped by cancelling all outstanding asynchronous
  // operations. Once all operations have finished the io_service::run() call
  // will exit.
  tcp_acceptor_.close();

#ifdef HTTP_WITH_SSL
  ssl_acceptor_.close();
#endif // HTTP_WITH_SSL

  connection_manager_.stopAll();
}

void Server::expireSessions(boost::system::error_code ec)
{
  LOG_DEBUG_S(&wt_, "expireSession()" << ec.message());

  if (!ec) {
    if (!wt_.expireSessions())
      wt_.scheduleStop();
    else {
      expireSessionsTimer_.expires_from_now(boost::posix_time::seconds(SESSION_EXPIRE_INTERVAL));
      expireSessionsTimer_.async_wait(boost::bind(&Server::expireSessions, this,
						  asio::placeholders::error));
    }
  } else if (ec != asio::error::operation_aborted) {
    LOG_ERROR_S(&wt_, "session expiration timer got an error: " << ec.message());
  }
}

} // namespace server
} // namespace http
