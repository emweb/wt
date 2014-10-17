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

namespace Wt {
  LOGGER("wthttp/async");
}

namespace http {
namespace server {

SslConnection::SslConnection(asio::io_service& io_service, Server *server,
    asio::ssl::context& context,
    ConnectionManager& manager, RequestHandler& handler)
  : Connection(io_service, server, manager, handler),
    socket_(io_service, context),
    sslShutdownTimer_(io_service)
{
  // avoid CRIME attack, get A rating on SSL analysis tools
#ifdef SSL_OP_NO_COMPRESSION
#if BOOST_VERSION >= 104700
  SSL_set_options(socket_.native_handle(), SSL_OP_NO_COMPRESSION);
#else
  SSL_set_options(socket_.impl()->ssl, SSL_OP_NO_COMPRESSION);
#endif
#endif
}

asio::ip::tcp::socket& SslConnection::socket()
{
  return socket_.next_layer();
}

void SslConnection::start()
{
  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());

  socket_.async_handshake(asio::ssl::stream_base::server,
			  strand_.wrap
			  (boost::bind(&SslConnection::handleHandshake,
			               sft,
				       asio::placeholders::error)));
}

void SslConnection::handleHandshake(const asio_error_code& error)
{
  SSL* ssl = 0;
#if BOOST_VERSION >= 104700
  ssl = socket_.native_handle();
#else //BOOST_VERSION < 104700
  if(socket_.impl())
    ssl = socket_.impl()->ssl;
#endif //BOOST_VERSION >= 104700

  if (!error) {
    Connection::start();
    // ssl handle must be registered after calling start(), since start()
    // resets the structs
    registerSslHandle(ssl);
  } else {
    long sslState = SSL_get_verify_result(ssl);
    if (sslState != X509_V_OK) {
      LOG_INFO("OpenSSL error: " 
	       << X509_verify_cert_error_string(sslState));
    }

    LOG_INFO("SSL handshake error: " << error.message());
    ConnectionManager_.stop(shared_from_this());
  }
}

void SslConnection::stop()
{
  LOG_DEBUG(socket().native() << ": stop()");
  finishReply();
  LOG_DEBUG(socket().native() << ": SSL shutdown");

  Connection::stop();
  
  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());

  sslShutdownTimer_.expires_from_now(boost::posix_time::seconds(1));
  sslShutdownTimer_.async_wait(strand_.wrap(
    boost::bind(&SslConnection::stopNextLayer,
    sft, asio::placeholders::error)));

  socket_.async_shutdown(strand_.wrap
			 (boost::bind(&SslConnection::stopNextLayer,
				      sft,
				      asio::placeholders::error)));
}

void SslConnection::stopNextLayer(const boost::system::error_code& ec)
{
  // We may get here either because of sslShutdownTimer_ timing out, or because
  // shutdown is complete. In both cases, closing next layer socket is the thing to do.
  // In case of timeout, we will get here twice.
  sslShutdownTimer_.cancel();
  if (ec) {
    LOG_DEBUG(socket().native() << ": ssl_shutdown failed:"
      << ec.message());
  }
  try {
    if (socket().is_open()) {
      boost::system::error_code ignored_ec;
      LOG_DEBUG(socket().native() << ": socket shutdown");
      socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      LOG_DEBUG(socket().native() << "closing socket");
      socket().close();
    }
  } catch (asio_system_error& e) {
    LOG_DEBUG(socket().native() << ": error " << e.what());
  }
}

void SslConnection::startAsyncReadRequest(Buffer& buffer, int timeout)
{
  if (state_ & Reading) {
    stop();
    return;
  }

  setReadTimeout(timeout);

  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (boost::bind(&SslConnection::handleReadRequestSsl,
				       sft,
				       asio::placeholders::error,
				       asio::placeholders::bytes_transferred)));
}

void SslConnection::handleReadRequestSsl(const asio_error_code& e,
                                         std::size_t bytes_transferred)
{
  // Asio SSL does not perform a write until the read handler
  // has returned. In the normal case, a read handler does not
  // return in case of a recursive event loop, so the SSL write
  // deadlocks a session. Hence, post the processing of the data
  // read, so that the read handler can return here immediately.
  strand_.post(boost::bind(&SslConnection::handleReadRequest,
			   shared_from_this(),
			   e, bytes_transferred));
}

void SslConnection::startAsyncReadBody(ReplyPtr reply,
				       Buffer& buffer, int timeout)
{
  if (state_ & Reading) {
    LOG_DEBUG(socket().native() << ": state_ = " << state_);
    stop();
    return;
  }

  setReadTimeout(timeout);

  boost::shared_ptr<SslConnection> sft
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (boost::bind(&SslConnection::handleReadBodySsl,
				       sft,
				       reply,
				       asio::placeholders::error,
				       asio::placeholders::bytes_transferred)));
}

void SslConnection::handleReadBodySsl(ReplyPtr reply,
				      const asio_error_code& e,
                                      std::size_t bytes_transferred)
{
  // See handleReadRequestSsl for explanation
  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  strand_.post(boost::bind(&SslConnection::handleReadBody,
			   sft, reply, e, bytes_transferred));
}

void SslConnection::startAsyncWriteResponse
    (ReplyPtr reply,
     const std::vector<asio::const_buffer>& buffers,
     int timeout)
{
  if (state_ & Writing) {
    LOG_DEBUG(socket().native() << ": state_ = " << state_);
    stop();
    return;
  }

  setWriteTimeout(timeout);

  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  asio::async_write(socket_, buffers,
		    strand_.wrap
		    (boost::bind(&SslConnection::handleWriteResponse,
				 sft,
				 reply,
				 asio::placeholders::error,
				 asio::placeholders::bytes_transferred)));
}

} // namespace server
} // namespace http

#endif // HTTP_WITH_SSL
