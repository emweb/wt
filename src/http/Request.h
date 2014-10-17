// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// request.hpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>

// For ::int64_ and ::uint64_t on Windows only
#include "Wt/WDllDefs.h"

#ifdef HTTP_WITH_SSL
#include <openssl/ssl.h>
#endif

namespace Wt {
  class WSslInfo;
}

namespace http {
namespace server {

/*
 * string that references data in our connection receive buffers,
 * taking into account that a string may span several buffer
 * boundaries
 */
struct buffer_string 
{
  char *data;
  unsigned int len;
  buffer_string *next;

  buffer_string() : data(0), len(0), next(0) { }

  bool empty() const { return len == 0 && (!next || next->empty()); }
  void clear() { data = 0; len = 0; next = 0; }
  std::string str() const;
  unsigned length() const;
  bool contains(const char *s) const;
  bool icontains(const char *s) const;
  bool iequals(const char *s) const;

  bool operator==(const buffer_string& other) const;
  bool operator==(const std::string& other) const;
  bool operator==(const char *other) const;
  bool operator!=(const char *other) const;
};

/// A request received from a client.
/// A request with a body will have a content-length.
class Request
{
public:
  struct Header {
    buffer_string name;
    buffer_string value;
  };

  Request() {
#ifdef HTTP_WITH_SSL
    ssl = 0;
#endif
  }
  enum State { Partial, Complete, Error };

  buffer_string method;
  buffer_string uri;
  char urlScheme[10];
  std::string remoteIP;
  short port;
  int http_version_major;
  int http_version_minor;

  typedef std::list<Header> HeaderList;
  HeaderList headers;
  ::int64_t contentLength;
  int webSocketVersion;

  enum Type {
    HTTP, // HTTP request
    WebSocket, // WebSocket request
    TCP // Raw TCP request (not interpreted by parser, e.g. for proxying)
  } type;

  std::string request_path;
  std::string request_query;
  std::string request_extra_path;

#ifdef HTTP_WITH_SSL
  SSL *ssl;
#endif
  Wt::WSslInfo *sslInfo() const;

  void reset();
  void process();

  bool closeConnection() const;
  bool acceptGzipEncoding() const;
  void enableWebSocket();
  const Header *getHeader(const std::string& name) const;
  const Header *getHeader(const char *name) const;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HPP
