// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SERVER_H_
#define SERVER_H_

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include "Configuration.h"

namespace Wt {
class WServer;
  namespace isapi {

class IsapiRequest;

class IsapiServer {
  IsapiServer();

public:
  ~IsapiServer();

  // to be called from the server thread when the WServer is properly started
  void setServerStarted();

  void serverEntry();

  void pushRequest(IsapiRequest *ecb);
  IsapiRequest *popRequest(int timeoutSec);

  void shutdown();

  static IsapiServer *instance();

  bool addServer(WServer *server);
  void removeServer(WServer *server);

  // Test to see if configuration() is non-zero before
  // invoking this log()!! So if (configuration()) log("notice") << "ok";
  WLogEntry log(const std::string& type);

  bool hasConfiguration() const { return server_ != 0; }
  Configuration &configuration() const;

  // IsapiServer takes ownership of the Configuration object and
  // will delete it on destruction. Once set, it must not be changed.
  //void setConfiguration(Configuration *c) { configuration_ = c; }

  // Provide access to the WServer object
  WServer *server() { return server_; }

private:
  static IsapiServer *instance_;

  std::thread serverThread_;

  std::mutex startedMutex_;
  std::condition_variable startedCondition_;
  bool started_;

  std::mutex queueMutex_;
  std::condition_variable queueCond_;
  std::deque<IsapiRequest *> queue_;

  // Also protected by queueMuex_;
  WServer *server_;
  // Once configuration_ is set, it remains valid until the IsapiServer is
  // destroyed.
  Configuration *configuration_;

  void setTerminated();
  bool terminated_;
};

}
}

#endif // WT_SERVER_H_
