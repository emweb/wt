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
#include <iostream>
#include <boost/thread/condition.hpp>

#include "WQApplication"
#include "DispatchThread.h"

namespace {
}

namespace Wt {

WQApplication::WQApplication(const WEnvironment& env, bool withEventLoop)
  : WApplication(env),
    withEventLoop_(withEventLoop),
    thread_(0)
{ }

void WQApplication::initialize()
{
  if (thread_)
    return;

  thread_ = new DispatchThread(this, withEventLoop_);
  thread_->start();
  thread_->waitDone();
}

void WQApplication::finalize()
{
  if (!thread_)
    return;

  thread_->destroy();

  delete thread_;
  thread_ = 0;
}

void WQApplication::notify(const WEvent& e)
{
  thread_->notify(e);
}

void WQApplication::realNotify(const WEvent& e)
{
  WApplication::notify(e);
}

WString toWString(const QString& s)
{
  return WString::fromUTF8((const char *)s.toUtf8());
}

QString toQString(const WString& s)
{
  return QString::fromUtf8(s.toUTF8().c_str());
}

}
