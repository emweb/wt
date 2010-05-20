#include "TimeUtil.h"

#ifdef _MSC_VER
#include "Winsock2.h"
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

namespace Wt {

Time::Time()
{
  t_ = new timeval;
  gettimeofday(t_, 0);
}

Time::Time(const Time &other)
{
   t_ = new timeval;
   memcpy(t_, other.t_, sizeof(timeval));
}

Time::~Time()
{
  delete t_;
  t_ = 0;
}

Time& Time::operator+= (int msec)
{
  t_->tv_sec += (msec / 1000);
  msec %= 1000;
  t_->tv_usec += (msec * 1000);
  t_->tv_sec += (t_->tv_usec / 1000000);
  t_->tv_usec %= 1000000;
  return *this;
}

Time& Time::operator= (const Time &other)
{
  if(&other != this) {
    memcpy(t_, other.t_, sizeof(timeval));
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
  int diffsec = t_->tv_sec - other.t_->tv_sec;
  int diffmsec = (t_->tv_usec - other.t_->tv_usec) / 1000;
  if (diffmsec < 0) {
    diffsec -= 1;
    diffmsec += 1000;
  }

  return diffsec * 1000 + diffmsec;
}

}
