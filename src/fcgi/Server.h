// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <string>
#include <vector>

#ifdef WT_THREADED
#include <thread>
#endif // WT_THREADED

namespace Wt {

class SessionInfo;
class WServer;

/*
 * A FastCGI relay server
 */
class Server
{
public:
  static bool bindUDStoStdin(const std::string& socketPath,
			     Wt::WServer& server);

  Server(WServer& wt,
         const std::string &applicationName,
         const std::vector<std::string> &args);
  int run();

  static Server *instance;

  void handleSigChld();
  void handleSignal(const char *signal);

private:
  WServer& wt_;
  std::string applicationName_;
  std::vector<std::string> args_;

  int childrenDied_;
  volatile sig_atomic_t handleSigChld_;

#ifdef WT_THREADED
  // mutex to protect access to the sessions map
  std::recursive_mutex mutex_;
#endif

  void checkAndQueueSigChld();
  void doHandleSigChld();

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
