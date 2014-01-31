// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SINGLE_THREADED_APPLICATION_H_
#define SINGLE_THREADED_APPLICATION_H_

#include <Wt/WApplication>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

/*
 * Base class for an application which handles events in a dedicated
 * thread.
 */
class SingleThreadedApplication : public Wt::WApplication
{
public:
  /*
   * Creates the application. The application is constructed from within
   * Wt's thread pool, and thus not yet in its private thread. You should
   * thus actually create the application from within the create() function.
   */
  SingleThreadedApplication(const Wt::WEnvironment& env);

protected:
  /*
   * Function running within the private thread which allows for application
   * construction.
   */
  virtual void create() = 0;

  /*
   * Function running within the private thread which allows for application
   * destruction.
   */
  virtual void destroy() = 0;

  /*
   * Actual notify within the private thread.
   */
  virtual void threadNotify(const Wt::WEvent& event);

  virtual void notify(const Wt::WEvent& event);
  virtual void initialize();
  virtual void finalize();
  virtual void waitForEvent();

private:
  boost::thread appThread_;
  bool finalized_, exception_;

  const Wt::WEvent *event_;

  boost::mutex     doneMutex_;
  bool             done_;
  boost::condition doneCondition_;

  boost::mutex     newEventMutex_;
  bool             newEvent_;
  boost::condition newEventCondition_;
  boost::mutex::scoped_lock *eventLock_;

  void run();
  void signalDone();
  void waitDone();
};

#endif // SINGLE_THREADED_APPLICATION_H_
