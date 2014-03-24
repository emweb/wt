/*
 * Copyright (C) 2014 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SingleThreadedApplication.h"

SingleThreadedApplication::
SingleThreadedApplication(const Wt::WEnvironment& env)
  : Wt::WApplication(env),
    finalized_(false),
    exception_(false),
    event_(0),
    done_(false),
    newEvent_(false)
{ }

void SingleThreadedApplication::initialize()
{
  log("debug") << "STA" << ": initialize()";
  Wt::WApplication::initialize();

  create();
}

void SingleThreadedApplication::finalize()
{
  log("debug") << "STA" << ": finalize()";
  Wt::WApplication::finalize();

  destroy();

  finalized_ = true;
}

void SingleThreadedApplication::notify(const Wt::WEvent& event)
{
  if (appThread_.get_id() == boost::thread::id()) { // not-a-thread
    done_ = false;
    log("debug") << "STA" << ": starting thread";
    appThread_ = boost::thread(boost::bind(&SingleThreadedApplication::run,
					   this));
    waitDone();
  }

  if (boost::this_thread::get_id() == appThread_.get_id()) {
    /* This could be from within a recursive event loop */
    log("debug") << "STA" << ": notify() called within app thread";
    threadNotify(event);
    return;
  }

  if (event.eventType() == Wt::ResourceEvent) {
    /*
     * We do not relay resource events since these will not unlock
     * a recursive event loop and thus we cannot communicate with the
     * private thread when it's blocked in a recursive event loop
     */
    log("debug") << "STA" << ": notify() for resource, handling in thread pool.";
    threadNotify(event);
    return;
  }

  event_ = &event;

  done_ = false;
  {
    log("debug") << "STA" << ": notifying thread";
    boost::mutex::scoped_lock lock(newEventMutex_);
    newEvent_ = true;
    newEventCondition_.notify_one();
  }

  waitDone();

  if (exception_) {
    exception_ = false;
    throw std::runtime_error("STA: rethrowing exception");
  }

  if (finalized_) {
    log("debug") << "STA" << ": joining thread";
    appThread_.join();
    appThread_ = boost::thread();
  }
}

void SingleThreadedApplication::waitDone()
{
  log("debug") << "STA" << ": waiting for event done";
  boost::mutex::scoped_lock lock(doneMutex_);

  while (!done_)
    doneCondition_.wait(lock);
}

void SingleThreadedApplication::run()
{
  signalDone();

  boost::mutex::scoped_lock lock(newEventMutex_);
  eventLock_ = &lock;

  for (;;) {
    if (!newEvent_) {
      log("debug") << "STA" << ": [thread] waiting for event";
      newEventCondition_.wait(lock);
    }

    log("debug") << "STA" << ": [thread] handling event";
    attachThread(true);
    try {
      threadNotify(*event_);
    } catch (std::exception& e) {
      log("error") << "STA" << ": [thread] Caught exception: " << e.what();
      exception_ = true;
    } catch (...) {
      log("error") << "STA" << ": [thread] Caught exception";
      exception_ = true;
    }
    attachThread(false);
    signalDone();

    if (finalized_)
      break;

    newEvent_ = false;
  }

  signalDone();
}

void SingleThreadedApplication::threadNotify(const Wt::WEvent& event)
{
  Wt::WApplication::notify(event);
}

void SingleThreadedApplication::waitForEvent()
{
  log("debug") << "STA" << ": [thread] waitForEvent()";

  eventLock_->unlock();
  try {
    Wt::WApplication::waitForEvent();
  } catch (...) {
    eventLock_->lock();
    throw;
  }

  eventLock_->lock();

  log("debug") << "STA" << ": [thread] returning from waitForEvent()";
}

void SingleThreadedApplication::signalDone()
{
  log("debug") << "STA" << ": [thread] signaling event done";
  boost::mutex::scoped_lock lock(doneMutex_);
  done_ = true;
  doneCondition_.notify_one();
}
