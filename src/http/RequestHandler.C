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
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

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

/*
 * Determine what to do with the request (based on the header),
 * and do it and create a Reply object which will do it.
 */
ReplyPtr RequestHandler::handleRequest(Request& req)
{
  if ((req.method != "GET")
      && (req.method != "HEAD")
      && (req.method != "POST")
      && (req.method != "PUT")
      && (req.method != "DELETE"))
    return ReplyPtr(new StockReply(req, Reply::not_implemented,
				   "", config_.errRoot()));

  if ((req.http_version_major != 1)
      || (req.http_version_minor != 0 
	  && req.http_version_minor != 1))
    return ReplyPtr(new StockReply(req, Reply::not_implemented, 
				   "", config_.errRoot()));

  // Decode url to path.
  if (!url_decode(req.uri, req.request_path, req.request_query)) {
    return ReplyPtr(new StockReply(req, Reply::bad_request,
				   "", config_.errRoot()));
  }

  std::size_t anchor = req.request_path.find("/#");
  if (anchor != std::string::npos) {
    // IE6 bug: it sends an anchor at the end of an URL '/' in AJAX requests.
    // We should therefore make sure we truncate here the path.
    req.request_path.erase(anchor + 1);
  }

  // Request path must be absolute and not contain "..".
  if (req.request_path.empty() || req.request_path[0] != '/'
      || req.request_path.find("..") != std::string::npos) {
    return ReplyPtr(new StockReply(req, Reply::bad_request,
				   "", config_.errRoot()));
  }

  for (unsigned i = 0; i < entryPoints_.size(); ++i) {
    const Wt::EntryPoint& ep = entryPoints_[i];

    bool matchesApp = false;

    // Check if path matches with the entry point's path (e.g. app.wt)
    // we should also match /app.wt/foobar.csv?bla=bo

    std::string pathInfo;
    unsigned entryPathLength = ep.path().length();

    if (boost::starts_with(req.request_path, ep.path())) {
      if (req.request_path.length() > entryPathLength) {
	char next = req.request_path[entryPathLength];
	if (next == '/') {
	  pathInfo = req.request_path.substr(entryPathLength);
	  matchesApp = true;	  
	}
      } else
	matchesApp = true;
    }

    if (matchesApp) {
      req.request_extra_path = pathInfo;
      if (!pathInfo.empty())
	req.request_path = ep.path();

      return ReplyPtr(new WtReply(req, ep, config_));
    }
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
  return ReplyPtr(new StaticReply(full_path, extension,
    req, config_.errRoot()));
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
