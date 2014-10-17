/*
 * Copyright (C) 2014 Emweb bvba, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "SessionProcessManager.h"

#include <boost/bind.hpp>

#ifndef WT_WIN32
#include <signal.h>
#include <sys/wait.h>
#endif

#if !defined(WT_WIN32) && BOOST_VERSION >= 104700
#define SIGNAL_SET
#endif

#include "Wt/WLogger"

namespace Wt {
  LOGGER("wthttp/proxy");
}

namespace http {
namespace server {

namespace {
  static const boost::shared_ptr<SessionProcess> nullProcessPtr
    = boost::shared_ptr<SessionProcess>();
#ifndef SIGNAL_SET
  static const int CHECK_CHILDREN_INTERVAL = 10;
#endif // SIGNAL_SET
}

SessionProcessManager::SessionProcessManager(boost::asio::io_service &ioService,
					     const Wt::Configuration &configuration)
  : 
#ifdef SIGNAL_SET
    signals_(ioService, SIGCHLD),
#else // !SIGNAL_SET
    timer_(ioService),
#endif // SIGNAL_SET
    numSessions_(0),
    configuration_(configuration)
{
#ifdef SIGNAL_SET
  signals_.async_wait(boost::bind(&SessionProcessManager::processDeadChildren, this,
	boost::asio::placeholders::error));
#else // !SIGNAL_SET
  timer_.expires_from_now(boost::posix_time::seconds(CHECK_CHILDREN_INTERVAL));
  timer_.async_wait(boost::bind(&SessionProcessManager::processDeadChildren, this,
	boost::asio::placeholders::error));
#endif // SIGNAL_SET
}

void SessionProcessManager::stop()
{
#ifdef SIGNAL_SET
  signals_.cancel();
#else // !SIGNAL_SET
  timer_.cancel();
#endif // SIGNAL_SET
}

bool SessionProcessManager::tryToIncrementSessionCount()
{
#ifndef SIGNAL_SET
  timer_.cancel();
#endif // SIGNAL_SET
  // Reap dead children, in case there are dead children,
  // and processDeadChildren hasn't been run yet.
  processDeadChildren(boost::system::error_code());
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(sessionsMutex_);
#endif // WT_THREADED
  if (numSessions_ + 1 > configuration_.maxNumSessions()) {
    return false;
  }
  ++ numSessions_;
  return true;
}

const boost::shared_ptr<SessionProcess>& SessionProcessManager::sessionProcess(std::string sessionId)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(sessionsMutex_);
#endif // WT_THREADED
  SessionMap::const_iterator it = sessions_.find(sessionId);
  if (it != sessions_.end()) {
    return it->second;
  }
  return nullProcessPtr;
}

void SessionProcessManager::addSessionProcess(std::string sessionId, const boost::shared_ptr<SessionProcess>& process)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(sessionsMutex_);
#endif // WT_THREADED
  if (!process->sessionId().empty()) {
    sessions_.erase(process->sessionId());
  }
  process->setSessionId(sessionId);
  sessions_[sessionId] = process;
}

std::vector<Wt::WServer::SessionInfo> SessionProcessManager::sessions() const
{
  std::vector<Wt::WServer::SessionInfo> result;
  for (SessionMap::const_iterator it = sessions_.begin();
       it != sessions_.end(); ++it) {
    Wt::WServer::SessionInfo sessionInfo;
#ifndef WT_WIN32
    sessionInfo.processId = it->second->pid();
#else // WT_WIN32
    sessionInfo.processId = it->second->processInfo().dwProcessId;
#endif // WT_WIN32
    sessionInfo.sessionId = it->first;
    result.push_back(sessionInfo);
  }
  return result;
}

void SessionProcessManager::processDeadChildren(boost::system::error_code ec)
{
  if (ec) {
    if (ec != boost::system::errc::operation_canceled)
      LOG_ERROR("Error processing dead children: " << ec.message());
    return;
  }

#ifndef WT_WIN32
  pid_t cpid;
  int status;

  while ((cpid = waitpid(0, &status, WNOHANG)) > 0) {
    removeSessionForPid(cpid);
  }
#else // WT_WIN32
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(sessionsMutex_);
#endif // WT_THREADED

  std::vector<std::string> toErase;
  for (SessionMap::iterator it = sessions_.begin();
        it != sessions_.end(); ++it) {
          DWORD result = WaitForSingleObject(it->second->processInfo().hProcess, 0);
    if (result == WAIT_OBJECT_0) {
      toErase.push_back(it->first);
    }
  }

  for (std::vector<std::string>::iterator it = toErase.begin();
       it != toErase.end(); ++it) {
    LOG_INFO("Child process " << sessions_[*it]->processInfo().dwProcessId << " died, removing session " << *it
	<< " (#sessions: " << (sessions_.size() - 1) << ")");
    sessions_[*it]->stop();
    sessions_.erase(*it);
    -- numSessions_;
  }
#endif // WT_WIN32
#ifdef SIGNAL_SET
  signals_.async_wait(boost::bind(&SessionProcessManager::processDeadChildren, this,
	boost::asio::placeholders::error));
#else // !SIGNAL_SET
  timer_.expires_from_now(boost::posix_time::seconds(CHECK_CHILDREN_INTERVAL));
  timer_.async_wait(boost::bind(&SessionProcessManager::processDeadChildren, this,
	boost::asio::placeholders::error));
#endif // SIGNAL_SET
}

#ifndef WT_WIN32
void SessionProcessManager::removeSessionForPid(pid_t cpid)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(sessionsMutex_);
#endif // WT_THREADED
  for (SessionMap::iterator it = sessions_.begin();
       it != sessions_.end(); ++it) {
    if(it->second->pid() == cpid) {
      LOG_INFO("Child process " << cpid << " died, removing session " << it->first
	  << " (#sessions: " << (sessions_.size() - 1) << ")");
      it->second->stop();
      sessions_.erase(it);
      -- numSessions_;
      return;
    }
  }
}
#endif // WT_WIN32

} // namespace server
} // namespace http
