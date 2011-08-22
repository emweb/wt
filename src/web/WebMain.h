// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WEB_MAIN_H_
#define WT_WEB_MAIN_H_

#if defined(WT_THREADED) && !defined(WT_TARGET_JAVA)
#include <boost/thread.hpp>
#include "threadpool/threadpool.hpp"
#endif

#include "WebController.h"

namespace Wt {

/*
 * This controls a main loop for connectors which are not
 * event-driven. It reads incoming requests from a webstream, and
 * schedules the handling of them into a thread pool.
 *
 * It also deal with event scheduling.
 */
class WT_API WebMain
{
public:
  WebMain(Configuration& configuration, WAbstractServer *server,
	  WebStream *stream, std::string singleSessionId = std::string());
  ~WebMain();

  WebController& controller() { return controller_; }

  void run();
  void shutdown();

  void schedule(int milliSeconds, const boost::function<void()>& function);

private:
  WebController  controller_;
  WebStream     *stream_;
  std::string    singleSessionId_;

#ifdef WT_THREADED
  boost::mutex scheduledEventsMutex_;
  boost::condition newScheduledEvent_;
  boost::thread schedulerThread_;
  void schedulerThread();
#endif // WT_THREADED

  typedef std::vector<boost::function<void()> > FunctionList;
  std::map<Wt::WDateTime, FunctionList> scheduledEvents_;

  bool shutdown_;

#ifdef WT_THREADED
  boost::threadpool::pool threadPool_;
#else
  bool handlingRequest_;
#endif

  void scheduleInThreadPool(WebRequest *request);
};

}

#endif // WEB_MAIN_H_
