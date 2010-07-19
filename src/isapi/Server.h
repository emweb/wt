// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include <map>

#ifndef WT_TARGET_JAVA
#ifdef WT_THREADED
#include <boost/thread.hpp>
#include "threadpool/threadpool.hpp"
#endif
#endif // WT_TARGET_JAVA

#include "Configuration.h"

namespace Wt {
class WServer;
  namespace isapi {

class IsapiRequest;

class IsapiServer {
  IsapiServer();

public:
  ~IsapiServer();

  void serverEntry();

  void pushRequest(IsapiRequest *ecb);
  IsapiRequest *popRequest(int timeoutSec);

  void shutdown();

  static IsapiServer *instance();

  bool addServer(WServer *server);

private:
  static IsapiServer *instance_;
  //WebController controller_;

  boost::thread serverThread_;

  boost::mutex queueMutex_;
  boost::condition_variable queueCond_;
  std::deque<IsapiRequest *> queue_;

  // Also protected by queueMuex_;
  WServer *server_;
};

}
}

#endif // WT_SERVER_H_
