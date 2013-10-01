#include "TimeUtil.h"

#include <boost/version.hpp>
#if BOOST_VERSION >= 104700
// Define below to avoid linking to boost.chrono library
#define BOOST_CHRONO_HEADER_ONLY
#include <boost/chrono.hpp>
#endif

#ifndef BOOST_CHRONO_HAS_CLOCK_STEADY
#ifdef _MSC_VER
#include "winsock2.h"
#pragma comment (lib, "winmm.lib")
namespace {
#include <windows.h>
  int gettimeofday(struct timeval* tp, void* tzp)
  {
    DWORD t;
    t = timeGetTime();
    tp->tv_sec = t / 1000;
    tp->tv_usec = t % 1000;
    return 0;
  }
// Next line is to make std::max operational again after loading windows.h
#undef max
}
#else
#include <sys/time.h>
#include <string.h>
#endif
#endif

namespace Wt {
#ifndef BOOST_CHRONO_HAS_CLOCK_STEADY

class TimeImpl {
public:
  struct timeval tv;
};

Time::Time()
{
  t_ = new TimeImpl;
  gettimeofday(&t_->tv, 0);
}

Time::Time(const Time &other)
{
   t_ = new TimeImpl;
   memcpy(&t_->tv, &other.t_->tv, sizeof(timeval));
}

Time::~Time()
{
  delete t_;
  t_ = 0;
}

Time& Time::operator+= (int msec)
{
  t_->tv.tv_sec += (msec / 1000);
  msec %= 1000;
  t_->tv.tv_usec += (msec * 1000);
  t_->tv.tv_sec += (t_->tv.tv_usec / 1000000);
  t_->tv.tv_usec %= 1000000;
  return *this;
}

Time& Time::operator= (const Time &other)
{
  if(&other != this) {
    memcpy(&t_->tv, &other.t_->tv, sizeof(timeval));
  }
  return *this;
}

Time Time::operator+ (int msec) const
{
  Time result(*this);
  result += msec;
  return result;
}

int Time::operator- (const Time& other) const
{
  int diffsec = t_->tv.tv_sec - other.t_->tv.tv_sec;
  int diffmsec = (t_->tv.tv_usec - other.t_->tv.tv_usec) / 1000;
  if (diffmsec < 0) {
    diffsec -= 1;
    diffmsec += 1000;
  }

  return diffsec * 1000 + diffmsec;
}

#else

class TimeImpl {
public:
  boost::chrono::steady_clock::time_point tp;
};

Time::Time()
{
  t_ = new TimeImpl;
  t_->tp = boost::chrono::steady_clock::now();
}

Time::Time(const Time &other)
{
  t_ = new TimeImpl;
  t_->tp = other.t_->tp;
}

Time::~Time()
{
  delete t_;
  t_ = 0;
}

Time& Time::operator+= (int msec)
{
  t_->tp += boost::chrono::milliseconds(msec);
  return *this;
}

Time& Time::operator= (const Time &other)
{
  t_->tp = other.t_->tp;
  return *this;
}

Time Time::operator+ (int msec) const
{
  Time result(*this);
  result += msec;
  return result;
}

int Time::operator- (const Time& other) const
{
  boost::chrono::milliseconds m =
    boost::chrono::duration_cast<boost::chrono::milliseconds>
    (t_->tp - other.t_->tp);

  return m.count();
}

#endif

}
