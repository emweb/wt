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

class SessionInfo;

class Server
{
public:
  Server(int argc, char *argv[]);
  int main();

  static Server *instance;

  void handleSigChld();
  void handleSignal(const char *signal);

private:
  int argc_;
  char **argv_;
  Configuration conf_;

#ifdef WT_THREADED
  // mutex to protect access to the sessions map
  boost::recursive_mutex mutex_;

  boost::threadpool::pool threadPool_;
#endif

  void spawnSharedProcess();
  void execChild(bool debug, const std::string& extraArg);

  int  connectToSession(const std::string& sessionId,
			const std::string& socketPath,
			int maxTries);
  bool getSessionFromQueryString(const std::string& uri,
				 std::string& sessionId);
  void checkConfig();
  bool writeToSocket(int socket, const unsigned char *buf, int bufsize);

  /*
   * For DedicatedProcess session policy
   */
  typedef std::map<std::string, SessionInfo *> SessionMap;
  SessionMap sessions_;

  void handleRequestThreaded(int serverSocket);
  void handleRequest(int serverSocket);

  /*
   * For SharedProcess session policy
   */
  std::vector<int> sessionProcessPids_;

  const std::string socketPath(const std::string& sessionId);
};

}

#endif // WT_SERVER_H_
