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

//#define DEBUG_ASYNC(a) a  
#define DEBUG_ASYNC(a)

namespace Wt {
  LOGGER("SslConnection");
}

namespace http {
namespace server {

SslConnection::SslConnection(asio::io_service& io_service, Server *server,
    asio::ssl::context& context,
    ConnectionManager& manager, RequestHandler& handler)
  : Connection(io_service, server, manager, handler),
    socket_(io_service, context)
{ }

asio::ip::tcp::socket& SslConnection::socket()
{
  return socket_.next_layer();
}

void SslConnection::start()
{
  socket_.async_handshake(asio::ssl::stream_base::server,
      boost::bind(&SslConnection::handleHandshake, this,
		  asio::placeholders::error));
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

    DEBUG_ASYNC(std::cerr << socket().native() << "handleHandshake error: "
      << error.message() << "\n");
    ConnectionManager_.stop(shared_from_this());
  }
}

void SslConnection::stop()
{
  DEBUG_ASYNC(std::cerr << socket().native() << ": stop()" << std::endl);
  finishReply();
  DEBUG_ASYNC(std::cerr << socket().native() << ": SSL shutdown" << std::endl);
  
  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  socket_.async_shutdown(boost::bind(&SslConnection::stopNextLayer,
				     sft,
				     asio::placeholders::error));
}

void SslConnection::stopNextLayer(const boost::system::error_code& ec)
{
  if (ec) {
    DEBUG_ASYNC(std::cerr << socket().native() << ": ssl_shutdown failed:"
      << ec.message() << std::endl);
  }
  try {
    boost::system::error_code ignored_ec;
    DEBUG_ASYNC(std::cerr 
		<< socket().native() << ": socket shutdown" 
		<< std::endl);
    socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    DEBUG_ASYNC(std::cerr 
		<< socket().native() << "closing socket" 
		<< std::endl);
    socket().close();
  } catch (asio_system_error& e) {
    DEBUG_ASYNC(std::cerr 
		<< socket().native() << ": error " << e.what() 
		<< std::endl);
  }
}

void SslConnection::startAsyncReadRequest(Buffer& buffer, int timeout)
{
  setReadTimeout(timeout);

  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  boost::bind(&SslConnection::handleReadRequestSsl,
				      sft,
				      asio::placeholders::error,
				      asio::placeholders::bytes_transferred));
}

void SslConnection::handleReadRequestSsl(const asio_error_code& e,
                                         std::size_t bytes_transferred)
{
  // Asio SSL does not perform a write until the read handler
  // has returned. In the normal case, a read handler does not
  // return in case of a recursive event loop, so the SSL write
  // deadlocks a session. Hence, post the processing of the data
  // read, so that the read handler can return here immediately.
  server()->service().post(boost::bind(&Connection::handleReadRequest,
                                       shared_from_this(),
                                       e, bytes_transferred));
}

void SslConnection::startAsyncReadBody(Buffer& buffer, int timeout)
{
  setReadTimeout(timeout);

  boost::shared_ptr<SslConnection> sft
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer),
			  boost::bind(&SslConnection::handleReadBodySsl,
				      sft,
				      asio::placeholders::error,
				      asio::placeholders::bytes_transferred));
}

void SslConnection::handleReadBodySsl(const asio_error_code& e,
                                      std::size_t bytes_transferred)
{
  // See handleReadRequestSsl for explanation
  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  server()->service().post(boost::bind(&SslConnection::handleReadBody,
                                       sft,
                                       e, bytes_transferred));
}

void SslConnection::startAsyncWriteResponse
    (const std::vector<asio::const_buffer>& buffers, int timeout)
{
  setWriteTimeout(timeout);

  boost::shared_ptr<SslConnection> sft 
    = boost::dynamic_pointer_cast<SslConnection>(shared_from_this());
  asio::async_write(socket_, buffers,
	boost::bind(&Connection::handleWriteResponse,
		    sft,
		    asio::placeholders::error));
}

} // namespace server
} // namespace http

#endif // HTTP_WITH_SSL
