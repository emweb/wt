// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
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

#include <Wt/AsioWrapper/asio.hpp>

#ifdef WTHTTP_WITH_ZLIB
#include <zlib.h>
#endif

#include "Wt/WStringStream.h"
#include "Wt/WLogger.h"
#include "../web/Configuration.h"

#include "Buffer.h"
#include "WHttpDllDefs.h"
#include "Request.h"

namespace http {
namespace server {

namespace asio = Wt::AsioWrapper::asio;

class Configuration;
class Connection;
class Reply;

typedef std::shared_ptr<Connection> ConnectionPtr;
typedef std::shared_ptr<Reply> ReplyPtr;

class WTHTTP_API Reply : public std::enable_shared_from_this<Reply>
{
public:
  Reply(Request& request, const Configuration& config);
  virtual ~Reply();

  virtual void reset(const Wt::EntryPoint *ep);

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
    service_unavailable = 503,
    version_not_supported = 505
  };

  enum ws_opcode {
    continuation = 0x0,
    text_frame = 0x1,
    binary_frame = 0x2,
    connection_close = 0x8,
    ping = 0x9,
    pong = 0xA,
  };

  /*
   * Called after writing data.
   *
   * In case the write was successful, this may call send() again if
   * more data is immediately available.
   */
  virtual void writeDone(bool success);

  /*
   * Returns true if ready to read more.
   */
  virtual bool consumeData(const char *begin,
			   const char *end,
			   Request::State state) = 0;

  /*
   * Returns false on failure (should abort reading more)
   */
  virtual bool consumeWebSocketMessage(ws_opcode opcode,
				       const char* begin,
				       const char* end,
				       Request::State state);

  void setConnection(ConnectionPtr connection);
  bool nextWrappedContentBuffers(std::vector<asio::const_buffer>& result);
  bool nextBuffers(std::vector<asio::const_buffer>& result);
  bool closeConnection() const;
  void setCloseConnection() { closeConnection_ = true; }
  void detectDisconnect(const std::function<void()>& callback);


  void addHeader(const std::string name, const std::string value);

  void receive();
  void send();

  const Configuration& configuration() { return configuration_; }

  virtual void logReply(Wt::WLogger& logger);
  void setStatus(status_type status);
  status_type status() const { return status_; }

protected:
  Request& request_;
  const Configuration& configuration_;

  virtual std::string contentType() = 0;
  virtual std::string location();
  virtual ::int64_t contentLength() = 0;

  /*
   * Provides next data to send. Will be called after send() has been called.
   * Returns whether this is the last data for this request.
   */
  virtual bool nextContentBuffers(std::vector<asio::const_buffer>& result)
    = 0;

  void setRelay(ReplyPtr reply);
  ReplyPtr relay() const { return relay_; }

  static std::string httpDate(time_t t);

  ConnectionPtr connection() const { return connection_; }
  bool transmitting() const { return transmitting_; }
  asio::const_buffer buf(const std::string &s);

private:

  std::vector<std::pair<std::string, std::string> > headers_;

  ConnectionPtr connection_;

  status_type status_;
  bool transmitting_;
  bool closeConnection_;
  bool chunkedEncoding_;
  bool gzipEncoding_;

  ::int64_t contentSent_;
  ::int64_t contentOriginalSize_;

  ReplyPtr relay_;

  Wt::WStringStream buf_;
  Wt::WStringStream postBuf_;
  // don't use vector; on resize the strings in bufs_ are copied, causing the
  // pointers in the asio buffer lists to become invalid
  std::list<std::string> bufs_;


  bool encodeNextContentBuffer(std::vector<asio::const_buffer>& result,
			       int& originalSize, int& encodedSize);
#ifdef WTHTTP_WITH_ZLIB
  void initGzip();
  bool gzipBusy_;
  z_stream gzipStrm_;
#endif
};

typedef std::shared_ptr<Reply> ReplyPtr;

} // namespace server
} // namespace http

#endif // HTTP_REPLY_HPP
