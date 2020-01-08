// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP

#include <set>
#include "Connection.h" // On WIN32, must be before thread stuff
#ifdef WT_THREADED
#include <mutex>
#endif // WT_THREADED


namespace http {
namespace server {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class ConnectionManager
{
public:
  ConnectionManager() = default;

  ConnectionManager(const ConnectionManager&) = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;

  /// Add the specified connection to the manager and start it.
  void start(ConnectionPtr c);

  /// Stop the specified connection.
  void stop(ConnectionPtr c);

  /// Stop all connections.
  void stopAll();

private:
  /// The managed connections.
  std::set<ConnectionPtr> connections_;

#ifdef WT_THREADED
  /// Mutex to protect access to connections_
  std::mutex mutex_;
#endif // WT_THREADED
};

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_MANAGER_HPP
