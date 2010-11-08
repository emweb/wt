// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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

#include <boost/asio.hpp>
namespace asio = boost::asio;
typedef boost::system::error_code asio_error_code;
typedef boost::system::system_error asio_system_error;

#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Buffer.h"
#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "RequestParser.h"

namespace http {
namespace server {

class ConnectionManager;
class Server;

/// Represents a single connection from a client.
class Connection
  : public boost::enable_shared_from_this<Connection>,
    private boost::noncopyable
{
public:
  /// Construct a connection with the given io_service.
  explicit Connection(asio::io_service& io_service, Server *server,
      ConnectionManager& manager, RequestHandler& handler);

  /// Get the socket associated with the connection.
  virtual asio::ip::tcp::socket& socket() = 0;

  /// Start the first asynchronous operation for the connection.
  virtual void start();

  /// Stop all asynchronous operations associated with the connection.
  virtual void stop() = 0;

  /// Like CGI's Url scheme: http or https
  virtual std::string urlScheme() = 0;

  virtual ~Connection();

  Server *server() const { return server_; }

public: // huh?
  void handleWriteResponse(const asio_error_code& e);
  void handleWriteResponse();
  void startWriteResponse();
  void handleReadRequest(const asio_error_code& e,
			 std::size_t bytes_transferred);
  /// Process read buffer, reading request.
  void handleReadRequest0();
  void handleReadBody(const asio_error_code& e,
		      std::size_t bytes_transferred);
  void handleReadBody();
  bool readAvailable();

protected:
  void setTimeout(int seconds);

  /// The manager for this connection.
  ConnectionManager& ConnectionManager_;

  void finishReply();

private:
  /*
   * Asynchronoulsy reading a request
   */
  /// Start reading request.
  virtual void startAsyncReadRequest(Buffer& buffer, int timeout) = 0;

  /*
   * Asynchronoulsy reading a request body
   */
  virtual void startAsyncReadBody(Buffer& buffer, int timeout) = 0;
  void handleError(const asio_error_code& e);
  void sendStockReply(Reply::status_type code);

  /*
   * Asynchronoulsy writing a response
   */
  virtual void startAsyncWriteResponse
      (const std::vector<asio::const_buffer>& buffers, int timeout) = 0;

  /// The handler used to process the incoming request.
  RequestHandler& request_handler_;

  void cancelTimer();
  void timeout(const asio_error_code& e);

  /// Timer for reading data.
  asio::deadline_timer timer_;

  /// Current buffer data, from last operation.
  Buffer buffer_;
  std::size_t buffer_size_;
  Buffer::const_iterator remaining_;

  /// The incoming request.
  Request request_;

  /// The parser for the incoming request.
  RequestParser request_parser_;

  /// The reply to be sent back to the client.
  ReplyPtr reply_;

  /// The reply is complete.
  bool moreDataToSend_;

  /// The server that owns this connection
  Server *server_;
};

typedef boost::shared_ptr<Connection> ConnectionPtr;

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_HPP
