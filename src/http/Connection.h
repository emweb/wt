// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
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

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <Wt/AsioWrapper/asio.hpp>
#include <Wt/AsioWrapper/strand.hpp>
#include <Wt/AsioWrapper/steady_timer.hpp>

#include "Buffer.h"
#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "RequestParser.h"

#include "Wt/WFlags.h"

namespace http {
namespace server {

namespace asio = Wt::AsioWrapper::asio;

class ConnectionManager;
class Server;

/// Represents a single connection from a client.
class Connection : public std::enable_shared_from_this<Connection>
{
public:
  /// Construct a connection with the given io_service.
  Connection(asio::io_service& io_service, Server *server,
	     ConnectionManager& manager, RequestHandler& handler);

  Connection(const Connection& other) = delete;

  /// Get the socket associated with the connection.
  virtual asio::ip::tcp::socket& socket() = 0;

  /// Start the first asynchronous operation for the connection.
  virtual void start();

  void close();
  bool closed() const;

  /// Like CGI's Url scheme: http or https
  virtual const char *urlScheme() = 0;

  virtual ~Connection();

  Server *server() const { return server_; }
  Wt::AsioWrapper::strand& strand() { return strand_; }

  /// Stop all asynchronous operations associated with the connection.
  void scheduleStop();

#ifdef HTTP_WITH_SSL
  void registerSslHandle(SSL *ssl) { request_.ssl = ssl; }
#endif

  bool waitingResponse() const { return waitingResponse_; }
  void setHaveResponse() { haveResponse_ = true; }
  void startWriteResponse(ReplyPtr reply);

  void handleReadBody(ReplyPtr reply);
  void readMore(ReplyPtr reply, int timeout);
  bool readAvailable();

  // NOTE: detectDisconnect will only register one callback at a time,
  //       further calls to detectDisconnect are ignored
  void detectDisconnect(ReplyPtr reply,
			const std::function<void()>& callback);
  void asyncDetectDisconnect(ReplyPtr reply,
			     const std::function<void()>& callback);

protected:
  /// Get the native handle of the socket
#if (defined(WT_ASIO_IS_BOOST_ASIO) && BOOST_VERSION >= 106600) || (defined(WT_ASIO_IS_STANDALONE_ASIO) && ASIO_VERSION >= 101100)
  asio::ip::tcp::socket::native_handle_type native();
#else
  asio::ip::tcp::socket::native_type native();
#endif

  void handleWriteResponse0(ReplyPtr reply,
                            const Wt::AsioWrapper::error_code& e,
			    std::size_t bytes_transferred);
  void handleWriteResponse(ReplyPtr reply);
  void handleReadRequest(const Wt::AsioWrapper::error_code& e,
			 std::size_t bytes_transferred);
  /// Process read buffer, reading request.
  void handleReadRequest0();
  void handleReadBody0(ReplyPtr reply,
                       const Wt::AsioWrapper::error_code& e,
		       std::size_t bytes_transferred);

  void setReadTimeout(int seconds);
  void setWriteTimeout(int seconds);

  /// The manager for this connection.
  ConnectionManager& ConnectionManager_;

  Wt::AsioWrapper::strand strand_;

  void finishReply();

  enum State {
    Idle = 0x0,
    Reading = 0x1,
    Writing = 0x2
  };

  Wt::WFlags<State> state_;

  virtual void stop();

private:
  /*
   * Asynchronoulsy reading a request
   */
  virtual void startAsyncReadRequest(Buffer& buffer, int timeout) = 0;

  /*
   * Asynchronoulsy reading a request body
   */
  virtual void startAsyncReadBody(ReplyPtr reply, Buffer& buffer,
				  int timeout) = 0;

  /*
   * Asynchronoulsy writing a response
   */
  virtual void startAsyncWriteResponse(ReplyPtr reply,
                              const std::vector<asio::const_buffer>& buffers,
				       int timeout) = 0;

  /// Generic I/O error handling: closes the connection and cancels timers
  void handleError(const Wt::AsioWrapper::error_code& e);

  void sendStockReply(Reply::status_type code);

  /// The handler used to process the incoming request.
  RequestHandler& request_handler_;

  void cancelReadTimer();
  void cancelWriteTimer();

  void timeout(const Wt::AsioWrapper::error_code& e);
  void doTimeout();

  /// Timer for reading data.
  asio::steady_timer readTimer_, writeTimer_;

  /// Current request buffer data
  std::list<Buffer> rcv_buffers_;

  /// Size of last buffer and iterator for next request in last buffer
  std::size_t rcv_buffer_size_;
  char *rcv_remaining_;
  bool rcv_body_buffer_;

  /// The incoming request.
  Request request_;

  /// The parser for the incoming request.
  RequestParser request_parser_;

  /// Recycled reply pointers
  ReplyPtr lastWtReply_, lastProxyReply_, lastStaticReply_;

  /// The server that owns this connection
  Server *server_;

  /// Indicates that we're waiting for a response while invoking a
  /// Reply function and thus that Reply function should not start a
  /// write response but simply indicate haveResponse_
  bool waitingResponse_;

  /// Indicates that we can send a response
  bool haveResponse_;

  /// Indicates that the current response is finished (after the
  /// current write operation)
  bool responseDone_;

  std::function<void()> disconnectCallback_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_HPP
