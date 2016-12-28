// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bvba, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_SESSION_PROCESS_MANAGER_HPP
#define HTTP_SESSION_PROCESS_MANAGER_HPP

#include <boost/asio.hpp>

#include "../web/Configuration.h"
#include "SessionProcess.h"
#include "Wt/WServer.h"

#if BOOST_VERSION >= 104900 && defined(BOOST_ASIO_HAS_STD_CHRONO)
#include <boost/asio/steady_timer.hpp>
typedef boost::asio::steady_timer asio_timer;
#else
typedef boost::asio::deadline_timer asio_timer;
#endif

namespace http {
namespace server {

typedef std::map<std::string, std::shared_ptr<SessionProcess> > SessionMap;
typedef std::vector<std::shared_ptr<SessionProcess> > SessionProcessList;

/// For dedicated processes: maps session ids to child processes and their sockets
class SessionProcessManager
{
public:
  SessionProcessManager(boost::asio::io_service &ioService,
			const Wt::Configuration& configuration);

  SessionProcessManager(const SessionProcessManager&) = delete;
  SessionProcessManager &operator=(const SessionProcessManager&) = delete;

  void stop();

  bool tryToIncrementSessionCount();
  const std::shared_ptr<SessionProcess>& sessionProcess(std::string sessionId);
  void addPendingSessionProcess(const std::shared_ptr<SessionProcess>& process);
  void addSessionProcess(std::string sessionId,
			 const std::shared_ptr<SessionProcess>& process);

  std::vector<Wt::WServer::SessionInfo> sessions() const;

private:
  void processDeadChildren(boost::system::error_code ec);
#ifndef WT_WIN32
  void removeSessionForPid(pid_t cpid);
#endif

#ifdef WT_THREADED
  std::mutex sessionsMutex_;
#endif // WT_THREADED
  SessionProcessList pendingProcesses_; // Processes that have started up, but are not mapped to a session yet
  SessionMap sessions_;
#if !defined(WT_WIN32) && BOOST_VERSION >= 104700
  boost::asio::signal_set signals_;
#else
  asio_timer timer_;
#endif

  int numSessions_;
  const Wt::Configuration &configuration_;
};

} // namespace server
} // namespace http

#endif // HTTP_SESSION_PROCESS_MANAGER_HPP
