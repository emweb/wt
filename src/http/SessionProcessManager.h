// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_SESSION_PROCESS_MANAGER_HPP
#define HTTP_SESSION_PROCESS_MANAGER_HPP

#include "../web/Configuration.h"
#include "SessionProcess.h"
#include "Wt/WServer.h"

#include "Wt/AsioWrapper/steady_timer.hpp"

namespace http {
namespace server {

typedef std::map<std::string, std::shared_ptr<SessionProcess> > SessionMap;
typedef std::vector<std::shared_ptr<SessionProcess> > SessionProcessList;

/// For dedicated processes: maps session ids to child processes and their sockets
class SessionProcessManager
{
public:
  SessionProcessManager(asio::io_service &ioService,
                        const Wt::Configuration& configuration);

  SessionProcessManager(const SessionProcessManager&) = delete;
  SessionProcessManager &operator=(const SessionProcessManager&) = delete;

  void stop();

  bool tryToIncrementSessionCount();

  std::shared_ptr<SessionProcess> createSessionProcess();
  const std::shared_ptr<SessionProcess>& sessionProcess(std::string sessionId);
  void updateSessionId(std::string sessionId,
                       const std::shared_ptr<SessionProcess>& process);

  std::vector<Wt::WServer::SessionInfo> sessions() const;

  asio::io_service& ioService();

private:
  void processDeadChildren(Wt::AsioWrapper::error_code ec);
#ifndef WT_WIN32
  void logExit(pid_t cpid, int wstatus);
  void removeSessionForPid(pid_t cpid);
#endif

#ifdef WT_THREADED
  mutable std::mutex sessionsMutex_;
#endif // WT_THREADED
  SessionProcessList pendingProcesses_; // Processes that have started up, but are not mapped to a session yet
  SessionMap sessions_;

  asio::io_service &ioService_;
#if !defined(WT_WIN32)
  asio::signal_set signals_;
#else
  asio::steady_timer timer_;
#endif

  int numSessions_;
  const Wt::Configuration &configuration_;
};

} // namespace server
} // namespace http

#endif // HTTP_SESSION_PROCESS_MANAGER_HPP
