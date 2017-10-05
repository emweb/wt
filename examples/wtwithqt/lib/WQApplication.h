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
#ifndef WQAPPLICATION_H_
#define WQAPPLICATION_H_

#include <Wt/WApplication.h>
#include <thread>

#include "DispatchThread.h"

/*! \file WQApplication */

class QString;

namespace Wt {

/*! \class WQApplication WQApplication WQApplication
 *  \brief An application class that provides interopability between
 *         Wt and Qt.
 *
 * This class provides interopability between the Wt's multi threading
 * model and Qt's threading requirements for QObject. This is needed
 * because Qt's object model, which defines a hierarchy of QObjects,
 * requires that every QObject in the hierarchy is created from within
 * the same thread. In addition, Qt's signal/slot system is
 * thread-aware and behaves very differently when a signal is emitted
 * from within a different thread than the thread in which the
 * receiver object lives.
 *
 * Wt on the other hand does not guarantee that every event is
 * dispatched within the same thread. This is a side effect of the
 * fact that Wt uses thread pools in combination with asynchronous I/O
 * to be able to serve multiple connections simultaneously without
 * requiring a high number of threads.
 *
 * Therefore, you cannot manipulate a QObject hierarchy, or propagate
 * events using Qt's signal/slot system, in a multi-threaded Wt
 * application server, since this is likely to violate Qt's
 * thread/object assumptions, without taking precautions (as are
 * implemented in this application class).
 *
 * This class spawns a QThread that is dedicated to a single
 * application instance, and used for event handling, after your
 * application is constructed. You should not create any Qt objects
 * from the constructor, but rather from the create() method, which
 * runs within the context of this thread. Likewise, you should not
 * destroy Qt objects from the application destructor, but from the
 * destroy() method, which also runs in this thread.
 *
 * You may enable a Qt event loop in this QThread, by setting the
 * option in the constructor. In this way, you can use QTcpSocket and
 * other Qt classes which rely on the presence of an event loop. Note
 * that Qt requires that you instantiate a QApplication object before
 * you can use a Qt event loop (only one is needed per process, so it
 * may be shared between multiple Wt sessions). You need to do this
 * yourself, and a convenient location could be within your main()
 * function.
 */
class WQApplication : public WApplication
{
public:
  /*! \brief Constructor.
   *
   * Create a new application with Qt threading support.
   *
   * Set <i>enableQtEventLoop</i> if you wish to enable a Qt event
   * loop within the thread context, e.g. when you wish to use certain
   * non-GUI classes that require the presence of an event loop (such
   * as QTimer, QTcpSocket, ...).
   *
   * Note: you should not create Qt objects from within the
   * constructor. Instead, reimplement create(), which is called after
   * construction, from within the QThread.
   */
  WQApplication(const WEnvironment& env, bool enableQtEventLoop = false);

protected:

  /*! \brief Initialize Qt objects in your application within the
   *         QThread context.
   *
   * Reimplement this method to construct your Wt widget and Qt object
   * hierarchy within the context of the dedicatd QThread.
   *
   * This method is called from within the library after your
   * application is created.
   */
  virtual void create() = 0;

  /*! \brief Finalize your application within the QThread context.
   *
   * Reimplement this method to safely destroy Qt object hierarchy.
   *
   * This method is called from within the library before your
   * application is deleted.
   */
  virtual void destroy() = 0;

  /*! \brief Notify an event to the application within the QThread
   *         context.
   *
   * This method is the equivalent of WApplication::notify(), but runs
   * inside the QThread context. The default implementation simply
   * calls WApplication::notify().
   */
  virtual void realNotify(const WEvent& e);

  virtual void notify(const WEvent& e);
  virtual void initialize();
  virtual void finalize();
  virtual void waitForEvent();

private:
  bool withEventLoop_;
  std::unique_ptr<DispatchThread> thread_;
  bool finalized_;
  bool recursiveEvent_;

  friend class DispatchThread;
};

/*! \brief Conversion function from QString to WString
 *
 * Lossless conversion between these two unicode string classes.
 */
extern WString toWString(const QString& s);

/*! \brief Conversion function from WString to QString
 *
 * Lossless conversion between these two unicode string classes.
 */
extern QString toQString(const WString& s);

}

#endif // WQAPPLICATION_H_
