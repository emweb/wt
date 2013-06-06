/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "RequestHandler.h"

#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>

#include "Request.h"
#include "StaticReply.h"
#include "StockReply.h"
#include "WtReply.h"

namespace http {
namespace server {

RequestHandler::RequestHandler(const Configuration &config,
			       const Wt::EntryPointList& entryPoints,
			       Wt::WLogger& logger)
  : config_(config),
    entryPoints_(entryPoints),
    logger_(logger)
{ }

bool RequestHandler::matchesPath(const std::string& path,
				 const std::string& prefix,
				 bool matchAfterSlash,
				 std::string& rest)
{
  if (boost::starts_with(path, prefix)) {
    unsigned prefixLength = prefix.length();

    if (path.length() > prefixLength) {
      char next = path[prefixLength];

      if (next == '/') {
	rest = path.substr(prefixLength);
	return true; 
      } else if (matchAfterSlash) {
	char last = prefix[prefixLength - 1];

	if (last == '/') {
	  rest = path.substr(prefixLength);
	  return true;
	}
      }
    } else {
      rest = std::string();
      return true;
    }
  }

  return false;
}

/*
 * Determine what to do with the request (based on the header),
 * and do it and create a Reply object which will do it.
 */
ReplyPtr RequestHandler::handleRequest(Request& req)
{
  if ((req.method != "GET")
      && (req.method != "HEAD")
      && (req.method != "OPTIONS")
      && (req.method != "POST")
      && (req.method != "PUT")
      && (req.method != "DELETE"))
    return ReplyPtr(new StockReply(req, Reply::not_implemented, "", config_));

  if ((req.http_version_major != 1)
      || (req.http_version_minor != 0 
	  && req.http_version_minor != 1))
    return ReplyPtr(new StockReply(req, Reply::not_implemented, "", config_));

  // Decode url to path.
  if (!url_decode(req.uri, req.request_path, req.request_query)) {
    return ReplyPtr(new StockReply(req, Reply::bad_request,"", config_));
  }

  std::size_t anchor = req.request_path.find("/#");
  if (anchor != std::string::npos) {
    // IE6 bug: it sends an anchor at the end of an URL '/' in AJAX requests.
    // We should therefore make sure we truncate here the path.
    req.request_path.erase(anchor + 1);
  }

  bool isStaticFile = false;

  if (!config_.defaultStatic()) {
    for (unsigned i = 0; i < config_.staticPaths().size(); ++i) {
      std::string notused;

      if (matchesPath(req.request_path, config_.staticPaths()[i],
		     true, notused)) {
	isStaticFile = true;
	break;
      }
    }
  }

  if (!isStaticFile) {
    int bestMatch = -1;
    std::string bestPathInfo = req.request_path;

    for (unsigned i = 0; i < entryPoints_.size(); ++i) {
      const Wt::EntryPoint& ep = entryPoints_[i];

      std::string pathInfo;

      bool matchesApp = matchesPath(req.request_path,
				    ep.path(),
				    !config_.defaultStatic(),
				    pathInfo);

      if (matchesApp) {
	if (pathInfo.length() < bestPathInfo.length()) {
	  bestPathInfo = pathInfo;
	  bestMatch = i;
	}
      }
    }

    if (bestMatch != -1) {
      const Wt::EntryPoint& ep = entryPoints_[bestMatch];

      req.request_extra_path = bestPathInfo;
      if (!bestPathInfo.empty())
	req.request_path = ep.path();

      return ReplyPtr(new WtReply(req, ep, config_));
    }
  }

  // Request path for a static file must be absolute and not contain "..".
  if (req.request_path.empty() || req.request_path[0] != '/'
      || req.request_path.find("..") != std::string::npos) {
    return ReplyPtr(new StockReply(req, Reply::bad_request, "", config_));
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (req.request_path[req.request_path.size() - 1] == '/') {
    req.request_path += "index.html";
  }

  // Determine the file extension.
  std::size_t last_slash_pos = req.request_path.find_last_of("/");
  std::size_t last_dot_pos = req.request_path.find_last_of(".");
  std::string extension;
  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
    extension = req.request_path.substr(last_dot_pos + 1);
  }

  std::string full_path = config_.docRoot() + req.request_path;
  return ReplyPtr(new StaticReply(full_path, extension, req, config_));
}

bool RequestHandler::url_decode(const std::string& in,
				std::string& path,
				std::string& query)
{
  std::string *out = &path;

  out->clear();
  out->reserve(in.size());

  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 2 < in.size())
      {
        int value;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          (*out) += static_cast<char>(value);
	  i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if ((in[i] == '?') && (out == &path))
    {
      query = in.substr(i + 1);
      return true;
    }
    else
    {
      (*out) += in[i];
    }
  }
  return true;
}

} // namespace server
} // namespace http
