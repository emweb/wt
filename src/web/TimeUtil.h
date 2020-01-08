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
  Time(); // now
  Time(const Time &other);

  Time operator+ (int msec) const;
  Time& operator+= (int msec);
  Time& operator= (const Time &other);

  int operator- (const Time& other) const; // milliseconds

private:
  std::chrono::steady_clock::time_point tp_;
};

}

#endif // TIMEUTIL_H_
