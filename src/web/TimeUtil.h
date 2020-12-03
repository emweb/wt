// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TIMEUTIL_H_
#define TIMEUTIL_H_

#include <chrono>
#include <Wt/WDllDefs.h>

struct timeval;

namespace Wt {

  class TimeImpl;

class WT_API Time
{
public:
  Time() noexcept; // now
#ifdef WT_TARGET_JAVA
  Time(const Time &other);
#endif // WT_TARGET_JAVA

  Time operator+ (int msec) const;
  Time& operator+= (int msec);

#ifdef WT_TARGET_JAVA
  Time& operator= (const Time &other);
#endif

  int operator- (const Time& other) const; // milliseconds

private:
  std::chrono::steady_clock::time_point tp_;
};


}

#endif // TIMEUTIL_H_
