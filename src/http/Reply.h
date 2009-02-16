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

#include <string>
#include <vector>

#ifdef BOOST_ASIO
#include <boost/asio.hpp>
namespace asio = boost::asio;
#else
#include <asio.hpp>
#endif

#ifdef THREADED
#include <boost/thread/recursive_mutex.hpp>
#endif // THREADED

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/tuple/tuple.hpp>
#ifdef WTHTTP_WITH_ZLIB
#include <zlib.h>
#endif

#include "Wt/WLogger"

#include "Buffer.h"
#include "WHttpDllDefs.h"

#ifdef WIN32
struct tm* gmtime_r(const time_t* t, struct tm* r);
#endif


namespace http {
namespace server {

class Connection;
class Reply;
class Request;

typedef boost::shared_ptr<Reply> ReplyPtr;

class WTHTTP_API Reply : public boost::enable_shared_from_this<Reply>
{
public:
  Reply(const Request& request);
  virtual ~Reply();

  enum status_type
  {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  };

  virtual void consumeRequestBody(Buffer::const_iterator begin,
				  Buffer::const_iterator end,
				  bool endOfRequest) = 0;

  void setConnection(Connection *connection);
  bool nextBuffers(std::vector<asio::const_buffer>& result);
  bool closeConnection() const { return closeConnection_; }

  void addHeader(const std::string name, const std::string value);

  void setExpectMoreData(bool how);
  bool expectMoreData() const { return expectMoreData_; }
  void transmitMore();

  void logReply(Wt::WLogger& logger);

protected:
  const Request& request_;
  std::string remoteAddress_;
  std::string requestMethod_;
  std::string requestUri_;
  int requestMajor_, requestMinor_;

  virtual status_type responseStatus() = 0;
  virtual std::string contentType() = 0;
  virtual std::string location();
  virtual boost::intmax_t contentLength() = 0;

  virtual asio::const_buffer nextContentBuffer() = 0;

  void setRelay(ReplyPtr reply);

  asio::const_buffer emptyBuffer;

  static std::string httpDate(time_t t);

private:
  std::vector<std::pair<std::string, std::string> > headers_;

#ifdef THREADED
  boost::recursive_mutex  connectionMutex_;
#endif // THREADED

  Connection             *connection_;

#ifndef WIN32
  struct timeval startTime_;
#endif

  bool transmitting_;
  bool closeConnection_;
  bool chunkedEncoding_;
  bool gzipEncoding_;
  bool expectMoreData_;

  boost::intmax_t contentSent_;
  boost::intmax_t contentOriginalSize_;

  ReplyPtr relay_;
  std::list<std::string> bufs_;

  asio::const_buffer buf(const std::string s);

  void encodeNextContentBuffer(std::vector<asio::const_buffer>& result,
			       int& originalSize,
			       int& encodedSize);
#ifdef WTHTTP_WITH_ZLIB
  void initGzip();
  z_stream gzipstrm_;
#endif
};

typedef boost::shared_ptr<Reply> ReplyPtr;

} // namespace server
} // namespace http

#endif // HTTP_REPLY_HPP
