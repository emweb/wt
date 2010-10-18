/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "ConnectionManager.h"

#include <algorithm>
#include <boost/bind.hpp>

namespace http {
namespace server {

void ConnectionManager::start(ConnectionPtr c)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  connections_.insert(c);
#ifdef WT_THREADED
  lock.unlock();
#endif // WT_THREADED

  c->start();
}

void ConnectionManager::stop(ConnectionPtr c)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  std::set<ConnectionPtr>::iterator i = connections_.find(c);
  if(i != connections_.end()) {
    connections_.erase(i);
  } else {
#ifndef WIN32
    /*
     * Error you may get when multiple transmitMore() were outstanding
     * during server push, and the last one indicated that the connection
     * needed to be closed: as a consequence they will all try to close
     * the connection.
     */
    std::cerr << "ConnectionManager::stop(): oops - stopping again?"
	      << std::endl;
    return;
#endif // WIN32
  }
#ifdef WT_THREADED
  lock.unlock();
#endif // WT_THREADED

  /*
   * Note: access to the connection's reply ptr is not thread-safe in
   * this way (FIXME).
   */
  c->stop();
}

void ConnectionManager::stopAll()
{
  while(connections_.size()) {
    stop(*connections_.begin());
  }
}

} // namespace server
} // namespace http
