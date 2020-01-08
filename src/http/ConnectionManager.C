/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
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
#include "Wt/WLogger.h"

namespace Wt {
  LOGGER("wthttp/async");
}

namespace http {
namespace server {

void ConnectionManager::start(ConnectionPtr c)
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> lock{mutex_};
#endif // WT_THREADED

  connections_.insert(c);

  LOG_DEBUG("new connection (#" << connections_.size() << ")");

#ifdef WT_THREADED
  lock.unlock();
#endif // WT_THREADED

  c->start();
}

void ConnectionManager::stop(ConnectionPtr c)
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> lock{mutex_};
#endif // WT_THREADED

  std::set<ConnectionPtr>::iterator i = connections_.find(c);
  if(i != connections_.end()) {
    connections_.erase(i);
  } else {
#ifndef WT_WIN32
    /*
     * Error you may get when multiple transmitMore() were outstanding
     * during server push, and the last one indicated that the connection
     * needed to be closed: as a consequence they will all try to close
     * the connection.
     */
    /*
      LOG_DEBUG("ConnectionManager::stop(): oops - stopping again?");
    */
    return;
#endif // WIN32
  }

  LOG_DEBUG("removed connection (#" << connections_.size() << ")");

#ifdef WT_THREADED
  lock.unlock();
#endif // WT_THREADED

  c->scheduleStop();
}

void ConnectionManager::stopAll()
{
  for (;;) {
    ConnectionPtr ptr;

    {
#ifdef WT_THREADED
      std::unique_lock<std::mutex> lock{mutex_};
#endif // WT_THREADED

      if (connections_.size())
	ptr = *connections_.begin();
    }

    if (ptr)
      stop(ptr);
    else
      break;
  }
}

} // namespace server
} // namespace http
