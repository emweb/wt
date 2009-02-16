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

class SessionInfo;

class Server
{
public:
  Server(int argc, char *argv[]);
  int main();

  static Server *instance;

  void handleSigChld();
  void handleSigTerm();

private:
  int argc_;
  char **argv_;
  Configuration conf_;

  void spawnSharedProcess();
  void execChild(bool debug, const std::string& extraArg);

  int  connectToSession(const std::string& sessionId,
			const std::string& socketPath,
			int maxTries);
  bool getSessionFromQueryString(const std::string& uri,
				 std::string& sessionId);
  void checkConfig();

  /*
   * For DedicatedProcess session policy
   */
  typedef std::map<std::string, SessionInfo *> SessionMap;
  SessionMap sessions_;

  /*
   * For SharedProcess session policy
   */
  std::vector<int> sessionProcessPids_;

  const std::string socketPath(const std::string& sessionId);
};

}

#endif // WT_SERVER_H_
