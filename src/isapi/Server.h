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
  void removeServer(WServer *server);

  // Test to see if configuration() is non-zero before
  // invoking this log()!! So if (configuration()) log("notice") << "ok";
  WLogEntry log(const std::string& type);

  bool hasConfiguration() const { return server_ != 0; }
  Configuration &configuration() const;

  // IsapiServer takes ownership of the Configuration object and
  // will delete it on destruction. Once set, it must not be changed.
  //void setConfiguration(Configuration *c) { configuration_ = c; }

private:
  static IsapiServer *instance_;

  boost::thread serverThread_;

  boost::mutex queueMutex_;
  boost::condition_variable queueCond_;
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
