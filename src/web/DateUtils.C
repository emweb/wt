#include "DateUtils.h"

#include "Wt/cpp20/date.hpp"
#include "Wt/WStringStream.h"

namespace {
  inline void pad2(Wt::WStringStream& buf, int value) {
    if (value < 10)
      buf << '0';
    buf << value;
  }
}

namespace Wt {
  namespace DateUtils {
    void httpDateBuf(const std::chrono::system_clock::time_point& tp,
                     Wt::WStringStream& buf)
    {
      const auto days = Wt::cpp20::date::floor<Wt::cpp20::date::days>(tp);
      const auto ymd = Wt::cpp20::date::year_month_day(days);
      const auto weekday = Wt::cpp20::date::weekday(days);
      const auto timeOfDay = Wt::cpp20::date::floor<std::chrono::seconds>(tp - days);
      const auto hms = Wt::cpp20::date::hh_mm_ss<std::chrono::seconds>(timeOfDay);

      static const char dayOfWeekStr[7][4]
              = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
      static const char monthStr[12][4]
              = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

      // Wed, 15 Jan 2014 21:20:01 GMT
      buf << dayOfWeekStr[weekday.c_encoding()] << ", ";
      pad2(buf, static_cast<unsigned>(ymd.day()));
      buf << ' ';
      buf << monthStr[static_cast<unsigned>(ymd.month()) - 1] << ' '
          << static_cast<int>(ymd.year()) << ' ';

      pad2(buf, hms.hours().count());
      buf << ':';
      pad2(buf, hms.minutes().count());
      buf << ':';
      pad2(buf, hms.seconds().count());
      buf << " GMT";
    }

    std::string httpDate(const std::chrono::system_clock::time_point& tp)
    {
      Wt::WStringStream ss;
      httpDateBuf(tp, ss);
      return ss.str();
    }
  }
}
