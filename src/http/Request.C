/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Wt/WLogger"
#include "Request.h"

namespace Wt {
  LOGGER("wthttp");
}

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
  webSocketVersion = -1;
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

void Request::enableWebSocket()
{
  webSocketVersion = -1;

  HeaderMap::const_iterator i = headerMap.find("Connection");
  if (i != headerMap.end() && boost::icontains(i->second, "Upgrade")) {
    HeaderMap::const_iterator j = headerMap.find("Upgrade");
    if (j != headerMap.end() && boost::iequals(j->second, "WebSocket")) {
      webSocketVersion = 0;

      HeaderMap::const_iterator k = headerMap.find("Sec-WebSocket-Version");
      if (k != headerMap.end()) {
	try {
	  webSocketVersion = boost::lexical_cast<int>(k->second);
	} catch (std::exception& e) {
	  LOG_ERROR("could not parse Sec-WebSocket-Version: " << k->second);
	}
      }
    }
  }
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

std::string Request::getHeader(const std::string& name) const
{
  HeaderMap::const_iterator i = headerMap.find(name);

  if (i != headerMap.end())
    return i->second;
  else
    return std::string();
}

} // namespace server
} // namespace http
