/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifdef WT_THREADED
#include <boost/bind.hpp>
#endif // WT_THREADED

#include <Wt/WAbstractServer>

#include "Configuration.h"
#include "WebMain.h"
#include "WebStream.h"

namespace Wt {

WebMain::WebMain(Configuration& configuration,
		 WAbstractServer *server, WebStream *stream,
		 std::string singleSessionId)
  : controller_(configuration, server, singleSessionId, false),
    stream_(stream),
    singleSessionId_(singleSessionId),
    shutdown_(false),
#ifdef WT_THREADED
    threadPool_(configuration.numThreads())
#else
    handlingRequest_(false)
#endif // WT_THREADED
{
  /*
   * FastCGI + default bootstrap will not work with only 1 thread.
   */
  if (configuration.serverType() == Configuration::FcgiServer
      && !configuration.progressiveBoot()
      && configuration.numThreads() == 1) {
    configuration.log("fatal")
      << "You need at least two threads configured in the config file, "
         "when running Wt with FastCGI and using default bootstrap mode.";
  }
}

WebMain::~WebMain()
{
#ifdef WT_THREADED
  if (schedulerThread_.joinable()) {
    boost::mutex::scoped_lock lock(scheduledEventsMutex_);
    scheduledEvents_.clear();
    newScheduledEvent_.notify_one();
  }
#endif // WT_THREADED
}

void WebMain::shutdown()
{
  shutdown_ = true;
  controller_.shutdown();
}

void WebMain::run()
{
  WebRequest *request = stream_->getNextRequest(10);

  if (request)
    controller_.server_->handleRequest(request);
  else
    if (!singleSessionId_.empty()) {
      controller_.configuration().log("error") << "No initial request ?";
      return;
    }

  for (;;) {
    bool haveMoreSessions = controller_.expireSessions();

    if (!haveMoreSessions && !singleSessionId_.empty()) {
      controller_.configuration().log("notice")
	<< "Dedicated session process exiting cleanly.";
      break;
    }

    WebRequest *request = stream_->getNextRequest(5);

    if (shutdown_) {
      controller_.configuration().log("notice")
	<< "Shared session server exiting cleanly.";
      break;
    }

    if (request)
      scheduleInThreadPool(request);

#ifndef WT_THREADED 
    while (!scheduledEvents_.empty()
	   && scheduledEvents_.begin()->first < WDateTime::currentDateTime()) {

      std::vector<boost::function<void()> > fs
	= scheduledEvents_.begin()->second;

      scheduledEvents_.erase(scheduledEvents_.begin());

      for (unsigned i = 0; i < fs.size(); ++i)
	fs[i]();
    }
#endif // WT_THREADED
  }
}

void WebMain::scheduleInThreadPool(WebRequest *request)
{
#ifdef WT_THREADED
  threadPool_.schedule
    (boost::bind(&WAbstractServer::handleRequest, controller_.server_, request));
#else
  controller_.server_->handleRequest(request);
#endif // WT_THREADED
}

void WebMain::schedule(int milliSeconds,
		       const boost::function<void()>& function)
{
#ifdef WT_THREADED
  if (milliSeconds == 0)
    threadPool_.schedule(function);
  else {
    boost::mutex::scoped_lock lock(scheduledEventsMutex_);
    scheduledEvents_[WDateTime::currentDateTime().addMSecs(milliSeconds)]
      .push_back(function);
    if (!schedulerThread_.joinable())
      schedulerThread_
	= boost::thread(boost::bind(&WebMain::schedulerThread, this));
    newScheduledEvent_.notify_one();
  }
#else
  scheduledEvents_[WDateTime::currentDateTime().addMSecs(milliSeconds)]
    .push_back(function);
#endif
}

#ifdef WT_THREADED
void WebMain::schedulerThread()
{
  for (;;) {
    boost::mutex::scoped_lock lock(scheduledEventsMutex_);

    while (!scheduledEvents_.empty()
	   && scheduledEvents_.begin()->first < WDateTime::currentDateTime()) {
      std::vector<boost::function<void()> > fs
	= scheduledEvents_.begin()->second;
      scheduledEvents_.erase(scheduledEvents_.begin());

      lock.unlock();
      for (unsigned i = 0; i < fs.size(); ++i)
	fs[i]();
      lock.lock();
    }

    if (scheduledEvents_.empty())
      newScheduledEvent_.wait(lock);
    else
      newScheduledEvent_.timed_wait
	(lock, scheduledEvents_.begin()->first.toPosixTime());

    if (scheduledEvents_.empty())
      return;
  }
}
#endif // WT_THREADED

}
