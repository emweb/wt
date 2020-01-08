// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SINGLE_THREADED_APPLICATION_H_
#define SINGLE_THREADED_APPLICATION_H_

#include <Wt/WApplication.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>


using namespace Wt;

/*
 * Base class for an application which handles events in a dedicated
 * thread.
 */
class SingleThreadedApplication : public WApplication
{
public:
  /*
   * Creates the application. The application is constructed from within
   * Wt's thread pool, and thus not yet in its private thread. You should
   * thus actually create the application from within the create() function.
   */
  SingleThreadedApplication(const WEnvironment& env);

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
  virtual void threadNotify(const WEvent& event);

  virtual void notify(const WEvent& event);
  virtual void initialize();
  virtual void finalize();
  virtual void waitForEvent();

private:
  std::thread     appThread_;
  bool finalized_, exception_;

  const WEvent                 *event_;

  std::mutex                    doneMutex_;
  bool                          done_;
  std::condition_variable       doneCondition_;

  std::mutex                    newEventMutex_;
  bool                          newEvent_;
  std::condition_variable       newEventCondition_;
  std::unique_lock<std::mutex> *eventLock_;

  void run();
  void signalDone();
  void waitDone();
};

#endif // SINGLE_THREADED_APPLICATION_H_
