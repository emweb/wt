#include "TimeUtil.h"

#include <chrono>

namespace Wt {

Time::Time() noexcept
{
  tp_ = std::chrono::steady_clock::now();
}

#ifdef WT_TARGET_JAVA
Time::Time(const Time &other)
{
  tp_ = other.tp_;
}
#endif // WT_TARGET_JAVA

Time& Time::operator+= (int msec)
{
  tp_ += std::chrono::milliseconds(msec);
  return *this;
}

#ifdef WT_TARGET_JAVA
Time& Time::operator= (const Time &other)
{
  tp_ = other.tp_;
  return *this;
}
#endif

Time Time::operator+ (int msec) const
{
  Time result(*this);
  result += msec;
  return result;
}

int Time::operator- (const Time& other) const
{
  std::chrono::milliseconds m =
    std::chrono::duration_cast<std::chrono::milliseconds>
    (tp_ - other.tp_);

  return m.count();
}

}
