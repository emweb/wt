// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>

#include "Wt/WLogger.h"

#include "Configuration.h"
#include "SessionProcessManager.h"
#include "WtReply.h"
#include "../web/Configuration.h"

namespace http {
namespace server {

class Configuration;
class Request;

/// The common handler for all incoming requests.
class RequestHandler
{
public:
  /// Construct with a directory containing files to be served.
  explicit RequestHandler(const Configuration &config,
			  const Wt::Configuration& wtConfig,
			  Wt::WLogger& logger);

  /// Handle a request and produce a reply.
  ReplyPtr handleRequest(Request& req, ReplyPtr& lastWtReply,
			 ReplyPtr& lastProxyReply,
			 ReplyPtr& lastStaticReply);

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  const std::string getErrorRoot() const
  {
    return config_.errRoot();
  }

  Wt::WLogger& logger() const { return logger_; }

  void setSessionManager(SessionProcessManager *sessionManager);

private:
  /// The server configuration
  const Configuration &config_;
  /// The Wt configuration
  const Wt::Configuration &wtConfig_;
  /// The logger
  Wt::WLogger& logger_;
  /// The session manager for dedicated processes
  SessionProcessManager *sessionManager_;

  /// Perform URL-decoding on a string and separates in path and
  /// query. Returns false if the encoding was invalid.
  static bool url_decode(const buffer_string& in, std::string& path,
			 std::string& query);

  static bool matchesPath(const std::string& path,
			  const std::string& prefix,
			  bool matchAfterSlash);

};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP
