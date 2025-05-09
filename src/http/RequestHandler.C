/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/utility/string_view.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include "Request.h"
#include "StaticReply.h"
#include "StockReply.h"
#include "WtReply.h"
#include "ProxyReply.h"

namespace {
  unsigned char fromHex(char b)
  {
    if (b <= '9')
      return b - '0';
    else if (b <= 'F')
      return (b - 'A') + 0x0A;
    else
      return (b - 'a') + 0x0A;
  }

  unsigned char fromHex(char msb, char lsb)
  {
    return (fromHex(msb) << 4) + fromHex(lsb);
  }
}

namespace http {
namespace server {

RequestHandler::RequestHandler(const Configuration &config,
                               const Wt::Configuration& wtConfig,
                               Wt::WLogger& logger)
  : config_(config),
    wtConfig_(wtConfig),
    logger_(logger),
    sessionManager_(nullptr)
{ }

void RequestHandler::setSessionManager(SessionProcessManager *sessionManager)
{
  sessionManager_ = sessionManager;
}

/*
 * Determine what to do with the request (based on the header),
 * and do it and create a Reply object which will do it.
 */
ReplyPtr RequestHandler::handleRequest(Request& req,
                                       ReplyPtr& lastWtReply,
                                       ReplyPtr& lastProxyReply,
                                       ReplyPtr& lastStaticReply)
{
  if ((req.method != "GET")
      && (req.method != "HEAD")
      && (req.method != "OPTIONS")
      && (req.method != "POST")
      && (req.method != "PUT")
      && (req.method != "DELETE")
      && (req.method != "PATCH"))
    return ReplyPtr(new StockReply(req, Reply::not_implemented, "", config_, wtConfig()));

  if ((req.http_version_major != 1)
      || (req.http_version_minor != 0
          && req.http_version_minor != 1))
    return ReplyPtr(new StockReply(req, Reply::version_not_supported, "", config_, wtConfig()));

  // Decode url to path.
  if (!url_decode(req.uri, req.request_path, req.request_query)) {
    return ReplyPtr(new StockReply(req, Reply::bad_request,"", config_, wtConfig()));
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
      if (Wt::Configuration::matchesPath(req.request_path, config_.staticPaths()[i], true)) {
        isStaticFile = true;
        break;
      }
    }
  }

  if (!isStaticFile) {
    Wt::EntryPointMatch bestMatch = wtConfig_.matchEntryPoint("", req.request_path, !config_.defaultStatic());

    if (bestMatch.entryPoint) {
      const Wt::EntryPoint& ep = *bestMatch.entryPoint;
      req.extra_start_index = bestMatch.extraStartIndex;
      req.url_params = std::move(bestMatch.urlParams);

      bool isRedirect = false;
      if (ep.type() != Wt::EntryPointType::StaticResource) {
        const auto pos = req.request_query.find("request=redirect");
        // request=redirect should be at the beginning or preceded by '&'
        if (pos != std::string::npos &&
            (pos == 0 || req.request_query[pos - 1] == '&')) {
          // request=redirect should be at the end or be followed by '&'
          const auto after = pos + (sizeof("request=redirect") - 1);
          if (after == req.request_query.size() ||
              req.request_query[after] == '&') {
            isRedirect = true;
          }
        }
      }

      if (wtConfig_.sessionPolicy() != Wt::Configuration::DedicatedProcess ||
          ep.type() == Wt::EntryPointType::StaticResource ||
          config_.parentPort() != -1 ||
          isRedirect) {
        if (!lastWtReply)
          lastWtReply.reset(new WtReply(req, ep, config_, wtConfig()));
        else
          lastWtReply->reset(&ep);

        return lastWtReply;
      } else {
        if (!lastProxyReply)
          lastProxyReply.reset(new ProxyReply(req, config_, *sessionManager_, wtConfig()));
        else
          lastProxyReply->reset(nullptr);

        return lastProxyReply;
      }

      // return ReplyPtr(new WtReply(req, ep, config_, , wtConfig()));
    }
  }

  if (!lastStaticReply)
    lastStaticReply.reset(new StaticReply(req, config_, wtConfig()));
  else
    lastStaticReply->reset(nullptr);

  return lastStaticReply;

  // return ReplyPtr(new StaticReply(req, config_, wtConfig()));
}

bool RequestHandler::url_decode(const buffer_string& in, std::string& path,
                                std::string& query)
{
  path.clear();

  std::string sin;
  const char *d;
  unsigned int len;

  if (in.next) {
    sin = in.str();
    d = sin.c_str();
    len = sin.length();
  } else {
    d = in.data;
    len = in.len;
  }

  // Only allow origin form (RFC 9112 3.2.1)
  if (!boost::starts_with(boost::string_view(d, len), "/"))
    return false;

  path.reserve(len);

  for (unsigned i = 0; i < len; ++i) {
    if (d[i] == '%') {
      if (i + 2 < len) {
        path += fromHex(d[i + 1], d[i + 2]);
        i += 2;
      } else
        return false;
    } else if (d[i] == '?') {
      query = std::string(d + i + 1, len - (i + 1));
      return true;
    } else
      path += d[i];
  }

  return true;
}

} // namespace server
} // namespace http
