/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WStringStream.h"
#include "Wt/WLocalDateTime.h"
#include "Wt/WLogger.h"
#include "Wt/WDateTime.h"
#include "Wt/WDate.h"
#include "Wt/WTime.h"

#include "Wt/Date/date.h"
#include "Wt/Date/tz.h"

#ifndef WT_WIN32
#include <ctime>
#else
#include <windows.h>
#endif

#include <chrono>
#include <type_traits>

namespace Wt {

  namespace {
    date::local_time<std::chrono::system_clock::time_point::duration>
    asLocalTime(const std::chrono::system_clock::time_point &dt)
    {
      return date::local_time<std::chrono::system_clock::time_point::duration>(dt.time_since_epoch());
    }
  }

LOGGER("WDateTime");

class WLocalDateTime::OffsetZone {
  std::chrono::minutes offset_;
  std::string name_;

public:
  explicit OffsetZone(std::chrono::minutes offset)
    : offset_{offset}
  {
    Wt::WStringStream ss;
    ss << "<custom zone, offset ";
    ss << std::string((offset < std::chrono::minutes{0}) ? "-" : "+");
    ss << static_cast<long long>(std::abs(offset.count()));
    ss << " minutes>";
    name_ = ss.str();
  }

  template<class Duration>
  date::local_time<typename std::common_type<Duration, std::chrono::minutes>::type>
  to_local(date::sys_time<Duration> tp) const
  {
    using LT = date::local_time<typename std::common_type<Duration, std::chrono::minutes>::type>;
    return LT{(tp + offset_).time_since_epoch()};
  }

  template<class Duration>
  date::sys_time<typename std::common_type<Duration, std::chrono::minutes>::type>
  to_sys(date::local_time<Duration> tp) const
  {
    using ST = date::sys_time<typename std::common_type<Duration, std::chrono::minutes>::type>;
    return ST{(tp - offset_).time_since_epoch()};
  }

  const std::string &name() const
  {
    return name_;
  }

  std::chrono::minutes offset() const
  {
    return offset_;
  }
};

WLocalDateTime::WLocalDateTime(const std::chrono::system_clock::time_point& dt,
			       const date::time_zone *zone, const WT_USTRING& format)
  : datetime_(dt),
    format_(format),
    zone_(zone),
    valid_(false),
    null_(false)
{
  if (!zone_)
    LOG_WARN("Invalid local date time: <no zone>");
  else
    valid_ = WDateTime(dt).isValid();
}

WLocalDateTime::WLocalDateTime(const std::chrono::system_clock::time_point& dt,
                               const std::shared_ptr<OffsetZone>& zone,
			       const WT_USTRING& format)
  : datetime_(dt),
    format_(format),
    zone_(nullptr),
    customZone_(zone),
    valid_(false),
    null_(false)
{
    valid_ = WDateTime(dt).isValid();
}

WLocalDateTime::WLocalDateTime(const WLocale& locale)
  : datetime_(std::chrono::system_clock::time_point()),
    format_(locale.dateTimeFormat()),
    zone_(locale.timeZone()),
    valid_(false),
    null_(true)
{ }

/*
 * Todo, add overload which indicates DST
 */
WLocalDateTime::WLocalDateTime(const WDate& date, const WTime& time,
			       const WLocale& locale)
  : datetime_(std::chrono::system_clock::time_point()),
    format_(locale.dateTimeFormat()),
    zone_(locale.timeZone()),
    valid_(false),
    null_(false)
{ 
  setDateTime(date, time);
}

WLocalDateTime WLocalDateTime::offsetDateTime(const std::chrono::system_clock::time_point& dt,
                                              std::chrono::minutes offset, const WT_USTRING& format)
{
  return WLocalDateTime(dt, std::make_shared<OffsetZone>(offset), format);
}

bool WLocalDateTime::isNull() const
{
  return null_;
}

bool WLocalDateTime::isValid() const
{
  return valid_;
}

void WLocalDateTime::setDateTime(const WDate& date, const WTime& time)
{
  null_ = false;
  valid_ = true;
  if (date.isValid() && time.isValid()) {
    if (zone_ || customZone_) {
      try {
        if (zone_)
          datetime_ = zone_->to_sys(asLocalTime(WDateTime(date, time).toTimePoint()));
        else
          datetime_ = customZone_->to_sys(asLocalTime(WDateTime(date, time).toTimePoint()));
      } catch(std::exception& e){
        LOG_WARN("Invalid local date time: " << e.what());
        setInvalid();
      }
    } else{
      LOG_WARN("Invalid local date time ("
               << date.toString() << " "
               << time.toString() << ") in zone "
               << "<no zone>");
      setInvalid();
    }
    if(isNull()){
      LOG_WARN("Invalid local date time ("
               << date.toString() << " "
               << time.toString() << ") in zone "
               << (zone_ ? zone_->name() : (customZone_ ? customZone_->name() : "<no zone>")));
      setInvalid();
    }

  } else
    setInvalid();
}

void WLocalDateTime::setInvalid()
{
  valid_ = false;
}

void WLocalDateTime::setDateTime(const WDate& date, const WTime& time,
				 bool dst)
{
  null_ = false;
  valid_ = true;
  if (date.isValid() && time.isValid()) {
    try {
      if (zone_) {
        if (dst)
          datetime_ = zone_->to_sys(asLocalTime(WDateTime(date, time).toTimePoint()), date::choose::latest);
        else
          datetime_ = zone_->to_sys(asLocalTime(WDateTime(date, time).toTimePoint()), date::choose::earliest);
        if (isNull()) {
          LOG_WARN("Invalid local date time ("
                   << date.toString() << " "
                   << time.toString() << " "
                   << "dst=" << dst << ") in zone "
                   << zone_->name());
          setInvalid();
        }
      } else if (customZone_) {
        datetime_ = customZone_->to_sys(asLocalTime(WDateTime(date, time).toTimePoint()));
        if (isNull()) {
          LOG_WARN("Invalid local date time ("
                   << date.toString() << " "
                   << time.toString() << " "
                   << "dst=" << dst << ") in zone "
                   << customZone_->name());
          setInvalid();
        }
      } else{
        LOG_WARN("Invalid local date time ("
                 << date.toString() << " "
                 << time.toString() << " "
                 << "dst=" << dst << ") in zone "
                 << "<no zone>");
        setInvalid();
      }
    } catch(std::exception& e) {
      LOG_WARN("Invalid local date time " << e.what());
      setInvalid();
    }
  } else
    setInvalid();
}

void WLocalDateTime::setDate(const WDate& date)
{
  if (isValid())
    setDateTime(date, time());
  else
    setDateTime(date, WTime(0, 0));
}

WDate WLocalDateTime::date() const
{
  if (isValid()){
    auto d = zone_ ? date::floor<date::days>(zone_->to_local(datetime_)) :
                     date::floor<date::days>(customZone_->to_local(datetime_));
    auto ymd = date::year_month_day(d);
    return WDate(int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()));
  }
  return WDate();
}

void WLocalDateTime::setTime(const WTime& time)
{
  if (isValid())
    setDateTime(date(), time);
}

WTime WLocalDateTime::time() const
{
  if (isValid()){
    auto dt = zone_ ? zone_->to_local(datetime_) : customZone_->to_local(datetime_);
    date::local_days dp = date::floor<date::days>(dt);
    auto time = date::make_time(dt - dp);
    std::chrono::duration<int, std::milli> ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.subseconds());
    return WTime(time.hours().count(), time.minutes().count(), time.seconds().count(), ms.count());
  }
  return WTime();
}

WDateTime WLocalDateTime::toUTC() const
{
  if (isValid()){
    return WDateTime(datetime_);
  }
  else
    return WDateTime();
}

WT_USTRING WLocalDateTime::toString() const
{
  return toString(format_);
}

int WLocalDateTime::timeZoneOffset() const
{
  if (zone_) {
    auto info = zone_->get_info(datetime_);
    return info.offset.count() / 60;
  } else if (customZone_) {
    return customZone_->offset().count();
  } else {
    throw WException("WLocalDateTime: timezone is null");
  }
}

const date::time_zone* WLocalDateTime::timeZone() const
{
  return zone_;
}

WT_USTRING WLocalDateTime::toString(const WT_USTRING& format) const
{
  WDate d = date();
  WTime t = time();
  return WDateTime::toString(&d, &t, format, true, timeZoneOffset());
}

WLocalDateTime WLocalDateTime::fromString(const WT_USTRING& s,
					  const WLocale& locale)
{
  WDateTime t = WDateTime::fromString(s, locale.dateTimeFormat());

  return WLocalDateTime(t.date(), t.time(), locale);
}

WLocalDateTime WLocalDateTime::currentDateTime(const WLocale& locale)
{
  WApplication *app = WApplication::instance();

  if (!locale.timeZone() && app)
    return currentTime(app->environment().timeZoneOffset(), locale.dateTimeFormat());
  else
    return WDateTime::currentDateTime().toLocalTime(locale);
}

WLocalDateTime WLocalDateTime::currentTime(std::chrono::minutes offset, const WT_USTRING& format)
{
  auto z = std::make_shared<OffsetZone>(offset);
  return WLocalDateTime(std::chrono::system_clock::now(), z, format);
}

WLocalDateTime WLocalDateTime::currentServerDateTime()
{
#ifndef WT_WIN32
  std::time_t t = std::time(nullptr);
  std::tm tm;
  ::localtime_r(&t, &tm);
  // tm_gmtoff is not part of the POSIX standard, but Linux, Mac OS X and the BSDs provide it
  return currentTime(date::floor<std::chrono::minutes>(std::chrono::seconds{tm.tm_gmtoff}),
		     WLocale::currentLocale().dateTimeFormat());
#else
  TIME_ZONE_INFORMATION tzi{};
  DWORD tz_result = ::GetTimeZoneInformation(&tzi);
  if (tz_result == TIME_ZONE_ID_INVALID)
  {
    return currentTime(std::chrono::minutes{0}, WLocale::currentLocale().dateTimeFormat());
  }
  bool dst = tz_result == TIME_ZONE_ID_DAYLIGHT;
  return currentTime(std::chrono::minutes{- tzi.Bias - (dst ? tzi.DaylightBias : 0)},
		     WLocale::currentLocale().dateTimeFormat());
#endif
}

bool WLocalDateTime::operator==(const WLocalDateTime& other) const
{
  return datetime_ == other.datetime_;
}

bool WLocalDateTime::operator!=(const WLocalDateTime& other) const
{
  return datetime_ != other.datetime_;
}

bool WLocalDateTime::operator<(const WLocalDateTime& other) const
{
  return datetime_ < other.datetime_;
}

}
