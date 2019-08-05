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

#include <Wt/WIOService.h>
#include <Wt/WServer.h>

#include "Server.h"
#include "Configuration.h"
#include "WebController.h"
#include "WebUtils.h"

#ifndef WT_WIN32
#include <unistd.h>
#include <fcntl.h>
#endif // WT_WIN32

namespace {
  bool parseAddressPort(const std::string &str,
                        const char *defaultPort,
                        std::string &address,
                        std::string &port)
  {
    if (str.empty()) {
      return false;
    } else {
      if (str[0] == '[') {
        std::size_t endIPv6 = str.find(']');
        if (endIPv6 == std::string::npos) {
          return false;
        } else {
          address = str.substr(1, endIPv6 - 1);
          if (endIPv6 != str.size() - 1) {
            if (str[endIPv6 + 1] == ':') {
              port = str.substr(endIPv6 + 2);
            } else {
              return false;
            }
          } else {
            port = defaultPort;
          }
          return true;
        }
      } else {
        std::size_t colonPos = str.find(':');
        if (colonPos == std::string::npos) {
          address = str;
          port = defaultPort;
          return true;
        } else {
          address = str.substr(0, colonPos);
          port = str.substr(colonPos + 1);
          return true;
        }
      }
    }
  }

  std::string addressString(const std::string &protocol,
                            const Wt::AsioWrapper::asio::ip::tcp::endpoint &ep,
                            const std::string &address)
  {
    std::string epAddrStr = ep.address().to_string();
    Wt::WStringStream ss;
    ss << protocol << "://";
    if (ep.address().is_v4())
      ss << epAddrStr;
    else if (ep.address().is_v6())
      ss << "[" << ep.address().to_string() << "]";
    ss << ":" << (int)ep.port();
    if (!address.empty() && epAddrStr != address) {
      ss << " (" << address << ")";
    }
    return ss.str();
  }

  std::string bindError(Wt::AsioWrapper::asio::ip::tcp::endpoint ep,
                        Wt::AsioWrapper::error_code errc) {
    Wt::AsioWrapper::system_error e{errc};
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
  SSL_CTX *nativeContext(Wt::AsioWrapper::asio::ssl::context& context)
  {
    return context.native_handle();
  }
#endif //HTTP_WITH_SSL

  // The interval to run WebController::expireSessions()
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
#ifdef HTTP_WITH_SSL
#if (defined(WT_ASIO_IS_BOOST_ASIO) && BOOST_VERSION >= 106600) || (defined(WT_ASIO_IS_STANDALONE_ASIO) && ASIO_VERSION >= 101100)
    ssl_context_(asio::ssl::context::sslv23),
#else
    ssl_context_(wt_.ioService(), asio::ssl::context::sslv23),
#endif
#endif // HTTP_WITH_SSL
    connection_manager_(),
    sessionManager_(0),
    request_handler_(config, wt_.configuration(), accessLogger_),
    expireSessionsTimer_(wt_.ioService())
{
  if (config.parentPort() != -1) {
    accessLogger_.configure(std::string("-*"));
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
  if (wt_.configuration().sessionPolicy() != Wt::Configuration::DedicatedProcess ||
      config_.parentPort() != -1) {
    // If we have one shared process, or this is the only session process,
    // run expireSessions() every SESSION_EXPIRE_INTERVAL seconds
    expireSessionsTimer_.expires_from_now
      (std::chrono::seconds(SESSION_EXPIRE_INTERVAL));
    expireSessionsTimer_.async_wait
      (std::bind(&Server::expireSessions, this, std::placeholders::_1));
  }

  asio::ip::tcp::resolver resolver(wt_.ioService());

  // HTTP
  if (config_.parentPort() != -1) {
    // address is always IPv4 loopback, port is picked automatically
    addTcpListener(resolver, "", "");
  } else {
    // Old style --http-address/--http-port
    if (!config_.httpAddress().empty())
      addTcpListener(resolver, config_.httpAddress(), config_.httpPort());

    // New style --http-listen
    for (std::size_t i = 0; i < config_.httpListen().size(); ++i) {
      const std::string &listenStr = config_.httpListen()[i];
      std::string address, port;
      if (parseAddressPort(listenStr, "80", address, port))
        addTcpListener(resolver, address, port);
      else
        throw Wt::WException(std::string("Could not bind to \"") + listenStr + "\": invalid format");
    }
  }

  // HTTPS
  if ((!config_.httpsAddress().empty() ||
       !config_.httpsListen().empty())
      && config_.parentPort() == -1) {
#ifdef HTTP_WITH_SSL
    // Configure SSL context
    if (config_.hasSslPasswordCallback())
      ssl_context_.set_password_callback(config_.sslPasswordCallback());

    long sslOptions = asio::ssl::context::default_workarounds
      | asio::ssl::context::no_sslv2
      | asio::ssl::context::single_dh_use;

    if (!config_.sslEnableV3())
      sslOptions |= asio::ssl::context::no_sslv3;

    sslOptions |= asio::ssl::context::no_tlsv1;
#if (defined(WT_ASIO_IS_BOOST_ASIO) && BOOST_VERSION >= 105800) || \
     defined(WT_ASIO_IS_STANDALONE_ASIO)
    sslOptions |= asio::ssl::context::no_tlsv1_1;
#endif

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
    
#if defined(SSL_CTX_set_ecdh_auto)
      SSL_CTX_set_ecdh_auto(native_ctx, 1);
#endif
    
    if (!config_.sslCipherList().empty()) {
      if (!SSL_CTX_set_cipher_list(native_ctx, config_.sslCipherList().c_str())) {
        throw Wt::WServer::Exception(
          "failed to select ciphers for cipher list "
          + config_.sslCipherList());
      }
    }
    
    if (config_.sslPreferServerCiphers()) {
      SSL_CTX_set_options(native_ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
    }

    std::string sessionId = Wt::WRandom::generateId(SSL_MAX_SSL_SESSION_ID_LENGTH);
    SSL_CTX_set_session_id_context(native_ctx,
      reinterpret_cast<const unsigned char *>(sessionId.c_str()), sessionId.size());
#else // HTTP_WITH_SSL
    LOG_ERROR_S(&wt_, "built without support for SSL: "
                "cannot start https server.");
#endif // HTTP_WITH_SSL
  }

#ifdef HTTP_WITH_SSL
  if (config_.parentPort() == -1) {
    // Old style --https-address/--https-port
    if (!config_.httpsAddress().empty())
      addSslListener(resolver, config_.httpsAddress(), config_.httpsPort());

    // New style --https-listen
    for (std::size_t i = 0; i < config_.httpsListen().size(); ++i) {
      const std::string &listenStr = config_.httpsListen()[i];
      std::string address, port;
      if (parseAddressPort(listenStr, "443", address, port))
        addSslListener(resolver, address, port);
      else
        throw Wt::WException(std::string("Could not bind to \"") + listenStr + "\": invalid format");
    }
  }
#endif // HTTP_WITH_SSL

  // Win32 cancels the non-blocking accept when the thread that called
  // accept exits. To avoid that this happens when called within the
  // WServer context, we post the action of calling accept to one of
  // the threads in the threadpool.
  wt_.ioService().post(std::bind(&Server::startAccept, this));

  if (config_.parentPort() != -1) {
    // This is a child process, connect to parent to
    // announce the listening port.
    std::shared_ptr<asio::ip::tcp::socket> parentSocketPtr
      (new asio::ip::tcp::socket(wt_.ioService()));
    wt_.ioService().post
      (std::bind(&Server::startConnect, this, parentSocketPtr));
  }
}

std::vector<asio::ip::address> Server::resolveAddress(asio::ip::tcp::resolver &resolver,
                                                      const std::string &address)
{
  std::vector<asio::ip::address> result;
  Wt::AsioWrapper::error_code errc;
  asio::ip::address fromStr = asio::ip::address::from_string(address, errc);
  if (!errc) {
    // The address is not a hostname, because it can be parsed as an
    // IP address, so we don't need to resolve it
    result.push_back(fromStr);
    return result;
  } else {
#ifndef NO_RESOLVE_ACCEPT_ADDRESS
    // Resolve IPv4
    asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), address, "http");
    asio::ip::tcp::resolver::iterator end;
    for (asio::ip::tcp::resolver::iterator it = resolver.resolve(query, errc);
         !errc && it != end; ++it) {
      result.push_back(it->endpoint().address());
    }
    if (errc)
      LOG_DEBUG_S(&wt_, "Failed to resolve hostname \"" << address << "\" as IPv4: " <<
                  Wt::AsioWrapper::system_error(errc).what());
    // Resolve IPv6
    query = Wt::AsioWrapper::asio::ip::tcp::resolver::query(Wt::AsioWrapper::asio::ip::tcp::v6(), address, "http");
    for (Wt::AsioWrapper::asio::ip::tcp::resolver::iterator it = resolver.resolve(query, errc);
         !errc && it != end; ++it) {
      result.push_back(it->endpoint().address());
    }
    if (errc)
      LOG_DEBUG_S(&wt_, "Failed to resolve hostname \"" << address << "\" as IPv6: " <<
                  Wt::AsioWrapper::system_error(errc).what());
    if (result.empty())
      LOG_WARN_S(&wt_, "Failed to resolve hostname \"" << address << "\": " <<
                 Wt::AsioWrapper::system_error(errc).what());
    return result;
#else // NO_RESOLVE_ACCEPT_ADDRESS
    LOG_WARN_S(&wt_, "Failed to resolve hostname \"" << address << "\": not supported");
    return result;
#endif
  }
}

Server::TcpListener::TcpListener(asio::ip::tcp::acceptor &&acceptor,
                                 TcpConnectionPtr new_connection)
  : acceptor(std::move(acceptor)), new_connection(new_connection)
{ }

void Server::addTcpListener(asio::ip::tcp::resolver &resolver,
                            const std::string &address,
                            const std::string &port)
{
  asio::ip::tcp::endpoint tcp_endpoint;
  Wt::AsioWrapper::error_code errc;

  if (config_.parentPort() == -1) {
    std::vector<asio::ip::address> addresses = resolveAddress(resolver, address);
    if (addresses.empty())
      throw Wt::WException(std::string("Could not bind to address ") + address + " port " + port
                           + ": Failed to resolve address.");
    bool couldBindToOne = false; // Tracks whether we could bind to at least one address
    for (std::vector<asio::ip::address>::const_iterator it = addresses.begin();
         it != addresses.end(); ++it) {
      tcp_endpoint.address(*it);
      if (port != "0")
        tcp_endpoint.port(atoi(port.c_str()));
      addTcpEndpoint(tcp_endpoint, address, errc);
      if (!errc)
        couldBindToOne = true;
    }
    if (!couldBindToOne)
      throw Wt::WException(std::string("Could not bind to address ") + address + " port " + port
                           + ": Could not listen on address.");
  } else {
    tcp_endpoint = asio::ip::tcp::endpoint(
          asio::ip::address_v4::loopback(), 0);
    addTcpEndpoint(tcp_endpoint, "", errc);
    if (errc)
      throw Wt::WException("Child process: failed to bind to IPv4 loopback address.");
  }
}

void Server::addTcpEndpoint(const asio::ip::tcp::endpoint &endpoint,
                            const std::string &address,
                            Wt::AsioWrapper::error_code &errc)
{
  tcp_listeners_.push_back(TcpListener(asio::ip::tcp::acceptor(wt_.ioService()), TcpConnectionPtr()));
  asio::ip::tcp::acceptor &tcp_acceptor = tcp_listeners_.back().acceptor;
  tcp_acceptor.open(endpoint.protocol());
  tcp_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
#ifndef WT_WIN32
  fcntl(tcp_acceptor.native_handle(), F_SETFD, fcntl(tcp_acceptor.native_handle(), F_GETFD) | FD_CLOEXEC);
#endif // WT_WIN32
  tcp_acceptor.bind(endpoint, errc);
  if (!errc) {
    tcp_acceptor.listen();

    LOG_INFO_S(&wt_, "started server: " << addressString("http", endpoint, address));

    tcp_listeners_.back().new_connection.reset
      (new TcpConnection(wt_.ioService(), this, connection_manager_,
                         request_handler_));
  } else {
    LOG_WARN_S(&wt_, bindError(endpoint, errc));
    tcp_listeners_.pop_back();
  }
}

#ifdef HTTP_WITH_SSL
Server::SslListener::SslListener(asio::ip::tcp::acceptor &&acceptor,
                                 SslConnectionPtr new_connection)
  : acceptor(std::move(acceptor)), new_connection(new_connection)
{ }

void Server::addSslListener(asio::ip::tcp::resolver &resolver,
                            const std::string &address,
                            const std::string &port)
{
  asio::ip::tcp::endpoint tcp_endpoint;
  Wt::AsioWrapper::error_code errc;

  std::vector<asio::ip::address> addresses = resolveAddress(resolver, address);
  if (addresses.empty())
    throw Wt::WException(std::string("Could not bind to address ") + address + " port " + port
                         + ": Failed to resolve address.");
  bool couldBindToOne = false; // Tracks whether we could bind to at least one address
  for (std::vector<asio::ip::address>::const_iterator it = addresses.begin();
       it != addresses.end(); ++it) {
    tcp_endpoint.address(*it);
    tcp_endpoint.port(atoi(port.c_str()));
    addSslEndpoint(tcp_endpoint, address, errc);
    if (!errc)
      couldBindToOne = true;
  }
  if (!couldBindToOne)
    throw Wt::WException(std::string("Could not bind to address ") + address + " port " + port
                         + ": Could not listen on address.");
}

void Server::addSslEndpoint(const asio::ip::tcp::endpoint &endpoint,
                            const std::string &address,
                            Wt::AsioWrapper::error_code &errc)
{
  ssl_listeners_.push_back(SslListener(asio::ip::tcp::acceptor(wt_.ioService()), SslConnectionPtr()));
  asio::ip::tcp::acceptor &ssl_acceptor = ssl_listeners_.back().acceptor;
  ssl_acceptor.open(endpoint.protocol());
  ssl_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
#ifndef WT_WIN32
  fcntl(ssl_acceptor.native_handle(), F_SETFD, fcntl(ssl_acceptor.native_handle(), F_GETFD) | FD_CLOEXEC);
#endif // WT_WIN32
  ssl_acceptor.bind(endpoint, errc);
  if (!errc) {
    ssl_acceptor.listen();

    LOG_INFO_S(&wt_, "started server: " << addressString("https", endpoint, address));

    ssl_listeners_.back().new_connection.reset
      (new SslConnection(wt_.ioService(), this, ssl_context_, connection_manager_,
                         request_handler_));
  } else {
    LOG_WARN_S(&wt_, bindError(endpoint, errc));
    ssl_listeners_.pop_back();
  }
}

#endif // HTTP_WITH_SSL

int Server::httpPort() const
{
  if (tcp_listeners_.empty()) {
#ifdef HTTP_WITH_SSL
    if (ssl_listeners_.empty())
      return -1;
    else
      return ssl_listeners_.front().acceptor.local_endpoint().port();
#else // HTTP_WITH_SSL
    return -1;
#endif // HTTP_WITH_SSL
  }

  return tcp_listeners_.front().acceptor.local_endpoint().port();
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
  for (std::size_t i = 0; i < tcp_listeners_.size(); ++i) {
    asio::ip::tcp::acceptor &acceptor = tcp_listeners_[i].acceptor;
    TcpConnectionPtr &new_connection = tcp_listeners_[i].new_connection;
    acceptor.async_accept(new_connection->socket(),
                          accept_strand_.wrap(
                            std::bind(&Server::handleTcpAccept, this,
                                        &tcp_listeners_[i],
                                        std::placeholders::_1)));
  }

#ifdef HTTP_WITH_SSL
  for (std::size_t i = 0; i < ssl_listeners_.size(); ++i) {
    asio::ip::tcp::acceptor &acceptor = ssl_listeners_[i].acceptor;
    SslConnectionPtr &new_connection = ssl_listeners_[i].new_connection;
    acceptor.async_accept(new_connection->socket(),
                          accept_strand_.wrap(
                            std::bind(&Server::handleSslAccept, this,
                                        &ssl_listeners_[i],
                                        std::placeholders::_1)));
  }
#endif // HTTP_WITH_SSL
}

void Server::startConnect(const std::shared_ptr<asio::ip::tcp::socket>& socket)
{
  socket->async_connect
    (asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(),
			     config_.parentPort()),
     std::bind(&Server::handleConnected, this, socket,
	       std::placeholders::_1));
}

void Server
::handleConnected(const std::shared_ptr<asio::ip::tcp::socket>& socket,
                  const Wt::AsioWrapper::error_code& err)
{
  if (!err) {
    // tcp_listeners_.front() should be fine here:
    // it should be the only acceptor when this is a session process
    std::shared_ptr<std::string> buf(new std::string(
      boost::lexical_cast<std::string>(tcp_listeners_.front().acceptor.local_endpoint().port())));
    socket->async_send(asio::buffer(*buf),
		       std::bind(&Server::handlePortSent, this, 
				 socket, err, buf));
  } else {
    LOG_ERROR_S(&wt_, "child process couldn't connect to parent to "
		"send listening port: " << err.message());
  }
}

void Server
::handlePortSent(const std::shared_ptr<asio::ip::tcp::socket>& socket,
                 const Wt::AsioWrapper::error_code& err,
		 const std::shared_ptr<std::string>& /*buf*/)
{
  if (err) {
    LOG_ERROR_S(&wt_, "child process couldn't send listening port: " 
		<< err.message());
  }

  Wt::AsioWrapper::error_code ignored_ec;
  if (socket.get()) {
    socket->shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
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
  wt_.ioService().post
    (accept_strand_.wrap(std::bind(&Server::handleStop, this)));
}

void Server::resume()
{
  wt_.ioService().post(std::bind(&Server::handleResume, this));
}

void Server::handleResume()
{
  for (std::size_t i = 0; i < tcp_listeners_.size(); ++i)
    tcp_listeners_[i].acceptor.close();
  tcp_listeners_.clear();

#ifdef HTTP_WITH_SSL
  for (std::size_t i = 0; i < ssl_listeners_.size(); ++i)
    ssl_listeners_[i].acceptor.close();
  ssl_listeners_.clear();
#endif // HTTP_WITH_SSL
  
  start();
}

void Server::handleTcpAccept(TcpListener *listener, const Wt::AsioWrapper::error_code& e)
{
  if (!e) {
    connection_manager_.start(listener->new_connection);
    listener->new_connection.reset(new TcpConnection(wt_.ioService(), this,
	  connection_manager_, request_handler_));
    listener->acceptor.async_accept(listener->new_connection->socket(),
                                    accept_strand_.wrap(
                                      std::bind(&Server::handleTcpAccept, this,
                                      listener, std::placeholders::_1)));
  }
}

#ifdef HTTP_WITH_SSL
void Server::handleSslAccept(SslListener *listener, const Wt::AsioWrapper::error_code& e)
{
  if (!e)
  {
    connection_manager_.start(listener->new_connection);
    listener->new_connection.reset(new SslConnection(wt_.ioService(), this,
          ssl_context_, connection_manager_, request_handler_));
    listener->acceptor.async_accept(listener->new_connection->socket(),
                                    accept_strand_.wrap(
                                      std::bind(&Server::handleSslAccept, this,
                                      listener, std::placeholders::_1)));
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
  for (std::size_t i = 0; i < tcp_listeners_.size(); ++i)
    tcp_listeners_[i].acceptor.close();
  tcp_listeners_.clear();

#ifdef HTTP_WITH_SSL
  for (std::size_t i = 0; i < ssl_listeners_.size(); ++i)
    ssl_listeners_[i].acceptor.close();
  ssl_listeners_.clear();
#endif // HTTP_WITH_SSL

  connection_manager_.stopAll();
}

void Server::expireSessions(Wt::AsioWrapper::error_code ec)
{
  LOG_DEBUG_S(&wt_, "expireSession()" << ec.message());

  if (!ec) {
    bool haveMoreSessions = wt_.expireSessions();
    if (!haveMoreSessions &&
	wt_.configuration().sessionPolicy() == Wt::Configuration::DedicatedProcess &&
	config_.parentPort() != -1)
      wt_.scheduleStop();
    else {
      expireSessionsTimer_.expires_from_now
        (std::chrono::seconds(SESSION_EXPIRE_INTERVAL));
      expireSessionsTimer_.async_wait
	(std::bind(&Server::expireSessions, this, std::placeholders::_1));
    }
  } else if (ec != asio::error::operation_aborted) {
    LOG_ERROR_S(&wt_, "session expiration timer got an error: " << ec.message());
  }
}

} // namespace server
} // namespace http
