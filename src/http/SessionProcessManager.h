// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bvba, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_SESSION_PROCESS_MANAGER_HPP
#define HTTP_SESSION_PROCESS_MANAGER_HPP

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "../web/Configuration.h"
#include "SessionProcess.h"
#include "Wt/WServer"

namespace http {
namespace server {

typedef std::map<std::string, boost::shared_ptr<SessionProcess> > SessionMap;

/// For dedicated processes: maps session ids to child processes and their sockets
class SessionProcessManager
  : private boost::noncopyable
{
public:
  SessionProcessManager(boost::asio::io_service &ioService, const Wt::Configuration& configuration);

  void stop();

  bool tryToIncrementSessionCount();
  const boost::shared_ptr<SessionProcess>& sessionProcess(std::string sessionId);
  void addSessionProcess(std::string sessionId, const boost::shared_ptr<SessionProcess>& connection);

  std::vector<Wt::WServer::SessionInfo> sessions() const;

private:
  void processDeadChildren(boost::system::error_code ec);
#ifndef WT_WIN32
  void removeSessionForPid(pid_t cpid);
#endif

#ifdef WT_THREADED
  boost::mutex sessionsMutex_;
#endif // WT_THREADED
  SessionMap sessions_;
#if !defined(WT_WIN32) && BOOST_VERSION >= 104700
  boost::asio::signal_set signals_;
#else
  boost::asio::deadline_timer timer_;
#endif

  int numSessions_;
  const Wt::Configuration &configuration_;
};

} // namespace server
} // namespace http

#endif // HTTP_SESSION_PROCESS_MANAGER_HPP
