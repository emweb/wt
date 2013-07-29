// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REPLY_HPP
#define HTTP_REPLY_HPP

#include <time.h>

#include <list>
#include <string>
#include <vector>

#include <boost/asio.hpp>
namespace asio = boost::asio;

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/tuple/tuple.hpp>
#ifdef WTHTTP_WITH_ZLIB
#include <zlib.h>
#endif

#include "Wt/WLogger"

#include "Buffer.h"
#include "WHttpDllDefs.h"
#include "Request.h"

namespace http {
namespace server {

class Configuration;
class Connection;
class Reply;

typedef boost::shared_ptr<Connection> ConnectionPtr;
typedef boost::shared_ptr<Reply> ReplyPtr;

typedef boost::weak_ptr<Connection> ConnectionWeakPtr;

class WTHTTP_API Reply : public boost::enable_shared_from_this<Reply>
{
public:
  Reply(const Request& request, const Configuration& config);
  virtual ~Reply();

  enum status_type
  {
    no_status = 0,
    switching_protocols = 101,
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    partial_content = 206,
    multiple_choices = 300,
    moved_permanently = 301,
    found = 302,
    see_other = 303,
    not_modified = 304,
    moved_temporarily = 307,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    request_entity_too_large = 413,
    requested_range_not_satisfiable = 416,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  };

  enum ws_opcode {
    continuation = 0x0,
    text_frame = 0x1,
    binary_frame = 0x2,
    connection_close = 0x8,
    ping = 0x9,
    pong = 0xA,
  };

  virtual void consumeData(Buffer::const_iterator begin,
			   Buffer::const_iterator end,
			   Request::State state) = 0;

  virtual void consumeWebSocketMessage(ws_opcode opcode,
				       Buffer::const_iterator begin,
				       Buffer::const_iterator end,
				       Request::State state);

  void setConnection(ConnectionPtr connection);
  bool nextBuffers(std::vector<asio::const_buffer>& result);
  bool closeConnection() const;
  void setCloseConnection() { closeConnection_ = true; }

  void addHeader(const std::string name, const std::string value);

  virtual bool waitMoreData() const { return false; }
  void send();

  const Configuration& configuration() { return configuration_; }

  void logReply(Wt::WLogger& logger);
  void setStatus(status_type status);
  status_type status() const { return status_; }

protected:
  const Request& request_;
  const Configuration& configuration_;
  std::string remoteAddress_;
  std::string requestMethod_;
  std::string requestUri_;
  int requestMajor_, requestMinor_;

  virtual std::string contentType() = 0;
  virtual std::string location();
  virtual ::int64_t contentLength() = 0;

  virtual void nextContentBuffers(std::vector<asio::const_buffer>& result) = 0;

  void setRelay(ReplyPtr reply);

  static std::string httpDate(time_t t);

  ConnectionPtr getConnection() { return connection_.lock(); }
  bool transmitting() const { return transmitting_; }

private:
  std::vector<std::pair<std::string, std::string> > headers_;

  ConnectionWeakPtr connection_;

  status_type status_;
  bool transmitting_;
  bool closeConnection_;
  bool chunkedEncoding_;
  bool gzipEncoding_;

  ::int64_t contentSent_;
  ::int64_t contentOriginalSize_;

  ReplyPtr relay_;
  std::list<std::string> bufs_;

  char gather_buf_[100];

  asio::const_buffer buf(const std::string s);

  void encodeNextContentBuffer(std::vector<asio::const_buffer>& result,
			       int& originalSize, int& encodedSize);
#ifdef WTHTTP_WITH_ZLIB
  void initGzip();
  bool gzipBusy_;
  z_stream gzipStrm_;
#endif
};

typedef boost::shared_ptr<Reply> ReplyPtr;

} // namespace server
} // namespace http

#endif // HTTP_REPLY_HPP
