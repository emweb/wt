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

  buffer_string() : data(nullptr), len(0), next(nullptr) { }

  bool empty() const { return len == 0 && (!next || next->empty()); }
  void clear() { data = nullptr; len = 0; next = nullptr; }
  std::string str() const;
  unsigned length() const;
  bool contains(const char *s) const;
  bool icontains(const char *s) const;
  bool iequals(const char *s) const;

  template<unsigned int N>
  bool istarts_with(const char (&str)[N]) const {
    return istarts_with(str, N-1);
  }

  void write(std::ostream &os) const;

  bool operator==(const buffer_string& other) const;
  bool operator==(const std::string& other) const;
  bool operator==(const char *other) const;
  bool operator!=(const char *other) const;

private:
  bool istarts_with(const char *s, unsigned int len) const;
};

std::ostream& operator<< (std::ostream &os, const buffer_string &str);

/// A request received from a client.
/// A request with a body will have a content-length.
class Request
{
public:
  struct Header {
    buffer_string name;
    buffer_string value;
  };
  
#ifdef WTHTTP_WITH_ZLIB
  struct PerMessageDeflateState {
	bool enabled;
	int client_max_window_bits; // -1 means no context takeover
	int server_max_window_bits; // -1 means no context takeover
  };
#endif

  Request() {
#ifdef HTTP_WITH_SSL
    ssl = nullptr;
#endif
    http_version_major = -1;
    http_version_minor = -1;
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
#ifdef WTHTTP_WITH_ZLIB
  mutable PerMessageDeflateState pmdState_;
#endif

  enum Type {
    HTTP, // HTTP request
    WebSocket, // WebSocket request
    TCP // Raw TCP request (not interpreted by parser, e.g. for proxying)
  } type;

  std::string request_path;
  std::string request_query;
  std::string request_extra_path;

  std::vector<std::pair<std::string, std::string> > url_params;

#ifdef HTTP_WITH_SSL
  SSL *ssl;
#endif
  std::unique_ptr<Wt::WSslInfo> sslInfo() const;

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
