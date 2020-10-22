/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "WQApplication.h"

#include "DispatchThread.h"

namespace Wt {

DispatchObject::DispatchObject(DispatchThread *thread)
  : thread_(thread)
{
  connect(this, SIGNAL(doEvent()), this, SLOT(onEvent()));
}

void DispatchObject::propagateEvent()
{
  emit doEvent();
}

void DispatchObject::onEvent()
{
  thread_->doEvent();
}

DispatchThread::DispatchThread(WQApplication *app,
			       bool withEventLoop)
  : QThread(),
    app_(app),
    qtEventLoop_(withEventLoop),
    event_(nullptr),
    exception_(false),
    done_(false),
    newEvent_(false),
    eventLock_(nullptr)
{ }

void DispatchThread::run()
{
  if (qtEventLoop_)
    dispatchObject_ = std::make_unique<DispatchObject>(this);

  signalDone();

  if (qtEventLoop_)
    exec();
  else
    myExec();

  dispatchObject_ = nullptr;

  signalDone();
}

void DispatchThread::myExec()
{
  std::unique_lock<std::mutex> lock(newEventMutex_);
  eventLock_ = &lock;

  for (;;) {
    if (!newEvent_) {
      log("debug") << "WQApplication: [thread] waiting for event";
      while (!newEvent_) {
        newEventCondition_.wait(lock);
      }
    }

    doEvent();

    if (app_->finalized_)
      return;

    newEvent_ = false;
  }
}

void DispatchThread::myPropagateEvent()
{
  {
    std::unique_lock<std::mutex> lock(newEventMutex_);
    newEvent_ = true;
  }
  newEventCondition_.notify_one();
}

void DispatchThread::signalDone()
{
  log("debug") << "WQApplication: [thread] signaling event done";
  {
    std::unique_lock<std::mutex> lock(doneMutex_);
    done_ = true;
  }
  doneCondition_.notify_one();
}

void DispatchThread::waitDone()
{
  std::unique_lock<std::mutex> lock(doneMutex_);

  while (!done_) {
    doneCondition_.wait(lock);
  }
 }

void DispatchThread::notify(const WEvent& event)
{
  if (event_) {
    app_->realNotify(event);
  } else {
    event_ = &event;

    done_ = false;

    if (dispatchObject_)
      dispatchObject_->propagateEvent();
    else
      myPropagateEvent();

    waitDone();

    event_ = 0;
  }
}

void DispatchThread::destroy()
{
  if (qtEventLoop_)
    QThread::exit();
}

void DispatchThread::resetException()
{
  exception_ = false;
}

void DispatchThread::doEvent()
{
  log("debug") << "WQApplication: [thread] handling event";
  app_->attachThread(true);

  try {
    app_->realNotify(*event_);
  } catch (std::exception& e) {
    log("error") << "WQApplication: [thread] Caught exception: " << e.what();
    exception_ = true;
  } catch (...) {
    log("error") << "WQApplication: [thread] Caught exception";
    exception_ = true;
  }
  app_->attachThread(false);
  log("debug") << "WQApplication: [thread] done handling event";

  signalDone();
}

}
