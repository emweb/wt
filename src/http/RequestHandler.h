// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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
#include <boost/noncopyable.hpp>

#include "Wt/WLogger"

#include "Configuration.h"
#include "Reply.h"
#include "../web/Configuration.h"

namespace http {
namespace server {

class Configuration;
class Request;

/// The common handler for all incoming requests.
class RequestHandler
  : private boost::noncopyable
{
public:
  /// Construct with a directory containing files to be served.
  explicit RequestHandler(const Configuration &config,
			  const Wt::EntryPointList& entryPoints,
			  Wt::WLogger& logger);

  /// Handle a request and produce a reply.
  ReplyPtr handleRequest(Request& req);

  const std::string getErrorRoot() const
  {
    return config_.errRoot();
  }

  Wt::WLogger& logger() const { return logger_; }

private:
  /// The server configuration
  const Configuration &config_;
  /// The paths that match applications
  const Wt::EntryPointList& entryPoints_;
  /// The logger
  Wt::WLogger& logger_;

  /// Perform URL-decoding on a string and separates in path and
  /// query. Returns false if the encoding was invalid.
  static bool url_decode(const std::string& in, std::string& path,
			 std::string& query);
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP
