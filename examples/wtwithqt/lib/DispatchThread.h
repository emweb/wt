// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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
#ifndef DISPATCH_THREAD_H_
#define DISPATCH_THREAD_H_

#include <QThread>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

namespace Wt {

class WQApplication;
class WEvent;
class DispatchThread;

/*
 * Help object used to dispatch an event into a Qt event loop.
 */
class DispatchObject : public QObject
{
  Q_OBJECT;

public:
  DispatchObject(DispatchThread *thread);

  void propagateEvent();

signals:
  void doEvent();

private slots:
  void onEvent();

private:
  DispatchThread *thread_;
};

/*
 * Thread in which all interaction with Qt objects is done.
 *
 * If constructed <i>withEventLoop</i>, then QThread::exec() is
 * called, starting a new Qt event loop, and signal/slot events can be
 * delivered within the event loop handling. Otherwise, plain thread
 * synchronization is implemented.
 */
class DispatchThread : public QThread
{
public:
  DispatchThread(WQApplication *app, bool withEventLoop);

  virtual void run();

  void notify(const WEvent& event);
  void destroy();

  void waitDone();

private:
  WQApplication    *app_;
  bool              qtEventLoop_;
  DispatchObject   *dispatchObject_;
  const WEvent     *event_;

  boost::mutex      doneMutex_;
  bool              done_;
  boost::condition  doneCondition_;

  boost::mutex      newEventMutex_;
  bool              newEvent_;
  boost::condition  newEventCondition_;

  bool doEvent();

  void signalDone();
  void myExec();
  void myPropagateEvent();

  friend class DispatchObject;
};

}

#endif // DISPATCH_THREAD_H_
