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

namespace http {
namespace server {

/*
 * boost::is_iless throws bad_cast -- here is my ad hoc version.
 */
struct my_iless
{
  bool operator()(const std::string& a, const std::string& b) const {
#if defined(WIN32) && !defined(__CYGWIN__)
    return _stricmp(a.c_str(), b.c_str()) < 0;
#else
    return strcasecmp(a.c_str(), b.c_str()) < 0;
#endif
  }
};

/// A request received from a client.
/// A request with a body will have a content-length.
class Request
{
public:
  enum State { Partial, Complete, Error };

  std::string method;
  std::string uri;
  std::string urlScheme;
  std::string remoteIP;
  short port;
  int http_version_major;
  int http_version_minor;

  typedef std::map<std::string, std::string, my_iless> HeaderMap;
  HeaderMap headerMap;
  std::vector<HeaderMap::iterator> headerOrder;
  ::int64_t contentLength;

  std::string request_path;
  std::string request_query;
  std::string request_extra_path;

  void reset();

  bool closeConnection() const;
  bool acceptGzipEncoding() const;
  bool isWebSocketRequest() const;

  void transmitHeaders(std::ostream& out) const;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HPP
