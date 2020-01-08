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

#ifdef HTTP_WITH_SSL

#include <vector>

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
  SSL_set_options(socket_.native_handle(), SSL_OP_NO_COMPRESSION);
#endif
}

asio::ip::tcp::socket& SslConnection::socket()
{
  return socket_.next_layer();
}

void SslConnection::start()
{
  std::shared_ptr<SslConnection> sft 
    = std::static_pointer_cast<SslConnection>(shared_from_this());

  socket_.async_handshake(asio::ssl::stream_base::server,
			  strand_.wrap
			  (std::bind(&SslConnection::handleHandshake,
				     sft,
				     std::placeholders::_1)));
}

void SslConnection::handleHandshake(const Wt::AsioWrapper::error_code& error)
{
  SSL* ssl = socket_.native_handle();

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
  LOG_DEBUG(native() << ": stop()");
  finishReply();
  LOG_DEBUG(native() << ": SSL shutdown");

  Connection::stop();
  
  std::shared_ptr<SslConnection> sft 
    = std::static_pointer_cast<SslConnection>(shared_from_this());

  sslShutdownTimer_.expires_from_now(std::chrono::seconds(1));
  sslShutdownTimer_.async_wait
    (strand_.wrap(std::bind(&SslConnection::stopNextLayer,
			    sft, std::placeholders::_1)));

  socket_.async_shutdown(strand_.wrap
			 (std::bind(&SslConnection::stopNextLayer,
				    sft,
				    std::placeholders::_1)));
}

void SslConnection::stopNextLayer(const Wt::AsioWrapper::error_code& ec)
{
  // We may get here either because of sslShutdownTimer_ timing out, or because
  // shutdown is complete. In both cases, closing next layer socket is the thing to do.
  // In case of timeout, we will get here twice.
  sslShutdownTimer_.cancel();
  if (ec) {
    LOG_DEBUG(native() << ": ssl_shutdown failed:"
      << ec.message());
  }
  try {
    if (socket().is_open()) {
      Wt::AsioWrapper::error_code ignored_ec;
      LOG_DEBUG(native() << ": socket shutdown");
      socket().shutdown(asio::ip::tcp::socket::shutdown_both, 
			ignored_ec);
      LOG_DEBUG(native() << "closing socket");
      socket().close();
    }
  } catch (Wt::AsioWrapper::system_error& e) {
    LOG_DEBUG(native() << ": error " << e.what());
  }
}

void SslConnection::startAsyncReadRequest(Buffer& buffer, int timeout)
{
  if (state_ & Reading) {
    stop();
    return;
  }

  setReadTimeout(timeout);

  std::shared_ptr<SslConnection> sft 
    = std::static_pointer_cast<SslConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (std::bind(&SslConnection::handleReadRequestSsl,
				     sft,
				     std::placeholders::_1,
				     std::placeholders::_2)));
}

void SslConnection::handleReadRequestSsl(const Wt::AsioWrapper::error_code& e,
                                         std::size_t bytes_transferred)
{
  // Asio SSL does not perform a write until the read handler
  // has returned. In the normal case, a read handler does not
  // return in case of a recursive event loop, so the SSL write
  // deadlocks a session. Hence, post the processing of the data
  // read, so that the read handler can return here immediately.
  strand_.post(std::bind(&SslConnection::handleReadRequest,
			 shared_from_this(),
			 e, bytes_transferred));
}

void SslConnection::startAsyncReadBody(ReplyPtr reply,
				       Buffer& buffer, int timeout)
{
  if (state_ & Reading) {
    LOG_DEBUG(native() << ": state_ = "
	      << (state_ & Reading ? "reading " : "")
	      << (state_ & Writing ? "writing " : ""));
    stop();
    return;
  }

  setReadTimeout(timeout);

  std::shared_ptr<SslConnection> sft
    = std::static_pointer_cast<SslConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  strand_.wrap
			  (std::bind(&SslConnection::handleReadBodySsl,
				     sft,
				     reply,
				     std::placeholders::_1,
				     std::placeholders::_2)));
}

void SslConnection::handleReadBodySsl(ReplyPtr reply,
                                      const Wt::AsioWrapper::error_code& e,
                                      std::size_t bytes_transferred)
{
  // See handleReadRequestSsl for explanation
  std::shared_ptr<SslConnection> sft 
    = std::static_pointer_cast<SslConnection>(shared_from_this());
  strand_.post(std::bind(&SslConnection::handleReadBody0,
			 sft, reply, e, bytes_transferred));
}

void SslConnection
::startAsyncWriteResponse(ReplyPtr reply,
			  const std::vector<asio::const_buffer>& buffers,
			  int timeout)
{
  if (state_ & Writing) {
    LOG_DEBUG(native() << ": state_ = "
	      << (state_ & Reading ? "reading " : "")
	      << (state_ & Writing ? "writing " : ""));
    stop();
    return;
  }

  setWriteTimeout(timeout);

  std::shared_ptr<SslConnection> sft 
    = std::static_pointer_cast<SslConnection>(shared_from_this());
  asio::async_write(socket_, buffers,
		    strand_.wrap
		    (std::bind(&SslConnection::handleWriteResponse0,
			       sft, reply,
			       std::placeholders::_1,
			       std::placeholders::_2)));
}

} // namespace server
} // namespace http

#endif // HTTP_WITH_SSL
