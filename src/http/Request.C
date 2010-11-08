/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Request.h"

namespace http {
namespace server {

void Request::reset()
{
  method.clear();
  uri.clear();
  urlScheme.clear();
  headerMap.clear();
  headerOrder.clear();
  request_path.clear();
  request_query.clear();

  contentLength = -1;
}

void Request::transmitHeaders(std::ostream& out) const
{
  static const char *CRLF = "\r\n";

  out << method << " " << uri << " HTTP/"
      << http_version_major << "."
      << http_version_minor << CRLF;

  for (std::size_t i = 0; i < headerOrder.size(); ++i) {
    HeaderMap::const_iterator it = headerOrder[i];
    out << it->first << ": " << it->second << CRLF;
  }
}

bool Request::isWebSocketRequest() const
{
  HeaderMap::const_iterator i = headerMap.find("Connection");
  if (i != headerMap.end() && boost::iequals(i->second, "Upgrade")) {
    HeaderMap::const_iterator j = headerMap.find("Upgrade");
    if (j != headerMap.end() && boost::iequals(j->second, "WebSocket"))
      return true;
  }

  return false;
}

bool Request::closeConnection() const 
{
  if ((http_version_major == 1)
      && (http_version_minor == 0)) {
    HeaderMap::const_iterator i = headerMap.find("Connection");

    if (i != headerMap.end()) {
      if (boost::iequals(i->second, "Keep-Alive"))
	return false;
    }

    return true;
  }

  if ((http_version_major == 1)
      && (http_version_minor == 1)) {
    HeaderMap::const_iterator i = headerMap.find("Connection");
    
    if (i != headerMap.end()) {
      if (boost::icontains(i->second, "close"))
	return true;
    }

    return false;
  }

  return true;
}

bool Request::acceptGzipEncoding() const
{
  HeaderMap::const_iterator i = headerMap.find("Accept-Encoding");

  if (i != headerMap.end())
    return i->second.find("gzip") != std::string::npos;
  else
    return false;
}

} // namespace server
} // namespace http
