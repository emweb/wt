/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdlib.h>

#include "Wt/WApplication.h"
#include "Wt/WDateTime.h"
#include "Wt/WDate.h"
#include "Wt/WLocalDateTime.h"
#include "Wt/WTime.h"
#include "Wt/Date/date.h"

#ifndef DOXYGEN_ONLY

namespace Wt {

  namespace {
    std::string multiple(int value, std::string s)
    {
      if (abs(value) == 1)
        return std::string();
      else
        return s;
    }
    inline WString tr(const char *key)
    {
      return WString::tr(key);
    }
    inline WString trn(const char *key, ::uint64_t n)
    {
      return WString::trn(key, n);
    }
  }

WDateTime::WDateTime()
  : invalid_date_time_(true), null_(true)
{ }

WDateTime::WDateTime(const WDate& date)
  : null_(false)
{
  if (date.isValid()) {
    datetime_ = date.toTimePoint();
    invalid_date_time_ = false;
  } else{
    invalid_date_time_ = true;
  }
}

WDateTime::WDateTime(const WDate& date, const WTime& time)
    : null_(false)
{
  if (date.isValid() && time.isValid()) {
    std::chrono::system_clock::time_point d = date.toTimePoint();
    d += std::chrono::hours(time.hour());
    d += std::chrono::minutes(time.minute());
    d += std::chrono::seconds(time.second());
    d += std::chrono::milliseconds(time.msec());
    datetime_ = d;
    invalid_date_time_ = false;
  } else
    invalid_date_time_ = true;
}

WDateTime::WDateTime(std::chrono::system_clock::time_point timepoint)
  : datetime_(timepoint), invalid_date_time_(false), null_(false)
{ }

void WDateTime::setTime_t(std::time_t t)
{
  null_ = false;
  datetime_ = std::chrono::system_clock::from_time_t(t);
  invalid_date_time_ = false;
}

void WDateTime::setTimePoint(const std::chrono::system_clock::time_point& timepoint)
{
  null_ = false;
  datetime_ = timepoint;
  invalid_date_time_ = false;
}

void WDateTime::setDate(const WDate& date)
{
  if (isValid())
    *this = WDateTime(date, time());
  else
    *this = WDateTime(date, WTime(0, 0));
}

const WDate WDateTime::date() const
{
  if (isValid()) {
    return WDate(datetime_);
  } else
    return WDate();
}

void WDateTime::setTime(const WTime& time)
{
  if (isValid())
    *this = WDateTime(date(), time);
}

const WTime WDateTime::time() const
{
  if(isValid()){
      date::sys_days dp = date::floor<date::days>(datetime_);
      auto time = date::make_time(datetime_ - dp);
      std::chrono::duration<int, std::milli> ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.subseconds());
      return WTime(time.hours().count(), time.minutes().count(), time.seconds().count(), ms.count());
  } else
      return WTime();
}

WDateTime WDateTime::addMSecs(int ms) const
{
  if(isValid()){
      std::chrono::system_clock::time_point dt = datetime_ + std::chrono::milliseconds(ms);
      return WDateTime(dt);
  } else
      return WDateTime();
}

WDateTime WDateTime::addSecs(int s) const
{
  if(isValid()){
      std::chrono::system_clock::time_point dt = datetime_ + std::chrono::seconds(s);
      return WDateTime(dt);
  } else
      return WDateTime();
}

WDateTime WDateTime::addDays(int ndays) const
{
  if(isValid()){
      WDate d = date().addDays(ndays);
      WTime t = time();
      return WDateTime(d, t);
  } else
      return WDateTime();
}

WDateTime WDateTime::addMonths(int nmonths) const
{
  if (isValid()) {
    WDate d = date().addMonths(nmonths);
    WTime t = time();
    return WDateTime(d, t);
  } else
    return WDateTime();
}

WDateTime WDateTime::addYears(int nyears) const
{
  if (isValid()) {
    WDate d = date().addYears(nyears);
    WTime t = time();
    return WDateTime(d, t);
  } else
    return WDateTime();
}

bool WDateTime::isNull() const
{
    return null_;
}

bool WDateTime::isValid() const
{
  return !invalid_date_time_;
}

std::time_t WDateTime::toTime_t() const
{
  if(isValid())
      return std::chrono::system_clock::to_time_t(datetime_);
  else
      return (std::time_t) 0;
}

std::chrono::system_clock::time_point WDateTime::toTimePoint() const
{
  return datetime_;
}

WLocalDateTime WDateTime::toLocalTime(const WLocale& locale) const
{
  if(isValid())
      return WLocalDateTime(datetime_, locale.timeZone(), locale.dateTimeFormat());
  return WLocalDateTime();
}

int WDateTime::secsTo(const WDateTime& other) const
{
  if (!isValid() || !other.isValid())
    return 0;

  return (int)other.toTime_t() - (int)toTime_t();
}

int WDateTime::daysTo(const WDateTime& other) const
{
  return date().daysTo(other.date());
}

WString WDateTime::timeTo(const WDateTime& other, std::chrono::seconds minValue) const
{
  if (!isValid() || !other.isValid())
    return WString::Empty;

  int secs = secsTo(other);

  if (abs(secs) < 1)
    if (WApplication::instance()) {
      return tr("Wt.WDateTime.LessThanASecond");
    } else {
      return "less than a second";
    }
  else if (abs(secs) < 60 * minValue.count())
    if (WApplication::instance()) {
      return trn("Wt.WDateTime.seconds", secs > 1 ? secs : 1).arg(secs);
    } else {
      return std::to_string(secs) + " second" + multiple(secs, "s");
    }
  else {
    int minutes = secs / 60;
    if (abs(minutes) < 60 * minValue.count())
      if (WApplication::instance()) {
	return trn("Wt.WDateTime.minutes", minutes > 1 ? minutes : 1).arg(minutes);
      } else {
        return std::to_string(minutes) + " minute" + multiple(minutes, "s");
      }
    else {
      int hours = minutes / 60;
      if (abs(hours) < 24 * minValue.count())
        if (WApplication::instance()) {
	  return trn("Wt.WDateTime.hours", hours > 1 ? hours : 1).arg(hours);
        } else {
          return std::to_string(hours) + " hour" + multiple(hours, "s");
        }
      else {
	int days = hours / 24;
        if (abs(days) < 7 * minValue.count())
          if (WApplication::instance()) {
	    return trn("Wt.WDateTime.days", days > 1 ? days : 1).arg(days);
	  } else {
            return std::to_string(days) + " day" + multiple(days, "s");
          }
	else {
	  if (abs(days) < 31 * minValue.count()) {
	    int weeks = days / 7;
            if (WApplication::instance()) {
	      return trn("Wt.WDateTime.weeks", weeks > 1 ? weeks : 1).arg(weeks);
            } else {
              return std::to_string(weeks) + " week" + multiple(weeks, "s");
            }
	  } else {
	    if (abs(days) < 365 * minValue.count()) {
	      int months = days / 30;
              if (WApplication::instance()) {
		return trn("Wt.WDateTime.months", months > 1 ? months : 1).arg(months);
              } else {
                return std::to_string(months) + " month"
		  + multiple(months, "s");
              }
	    } else {
	      int years = days / 365;
              if (WApplication::instance()) {
		return trn("Wt.WDateTime.years", years > 1 ? years : 1).arg(years);
	      } else {
		return std::to_string(years) + " year"
		  + multiple(years, "s");
	      }
	    }
	  }
	}
      }
    }
  }
}

bool WDateTime::operator<(const WDateTime& other) const
{
  return datetime_ < other.datetime_;
}

bool WDateTime::operator<=(const WDateTime& other) const
{
  return datetime_ <= other.datetime_;
}

bool WDateTime::operator>(const WDateTime& other) const
{
  return datetime_ > other.datetime_;
}

bool WDateTime::operator>=(const WDateTime& other) const
{
  return datetime_ >= other.datetime_;
}

bool WDateTime::operator==(const WDateTime& other) const
{
  return datetime_ == other.datetime_;
}

bool WDateTime::operator!=(const WDateTime& other) const
{
  return datetime_ != other.datetime_;
}

WDateTime WDateTime::currentDateTime()
{
  return WDateTime(std::chrono::system_clock::now());
}

WString WDateTime::defaultFormat()
{
  return WString::fromUTF8("ddd MMM d HH:mm:ss yyyy"); 
}

WDateTime WDateTime::fromString(const WString& s)
{
  return fromString(s, defaultFormat());
}

WDateTime WDateTime::fromString(const WString& s, const WString& format)
{
  WDate date;
  WTime time;

  fromString(&date, &time, s, format);

  return WDateTime(date, time);
}

void WDateTime::fromString(WDate *date, WTime *time, const WString& s,
			   const WString& format)
{
  std::string v = s.toUTF8();
  std::string f = format.toUTF8();

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  unsigned vi = 0;

  WDate::ParseState dateParse;
  WTime::ParseState timeParse;

  for (unsigned fi = 0; fi <= f.length(); ++fi) {
    bool finished = fi == f.length();
    char c = !finished ? f[fi] : 0;

    if (finished && inQuote)
      return;

    if (inQuote) {
      if (c != '\'') {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else {
	  if (vi >= v.length() || (v[vi++] != c))
	    return;
	}
      } else {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  if (vi >= v.length() || (v[vi++] != c))
	    return;
	} else {
	  gotQuoteInQuote = true;
	  inQuote = false;
	}
      }
    } else {
      CharState state = CharState::CharUnhandled;

      if (date) {
	CharState dateState = WDate::handleSpecial(c, v, vi, dateParse, format);
	if (dateState == CharState::CharInvalid)
	  return;
	else if (dateState == CharState::CharHandled)
	  state = CharState::CharHandled;
      }

      if (time) {
	CharState timeState = WTime::handleSpecial(c, v, vi, timeParse, format);
	if (timeState == CharState::CharInvalid)
	  return;
	else if (timeState == CharState::CharHandled)
	  state = CharState::CharHandled;
      }

      if (!finished && state == CharState::CharUnhandled) {
	if (c == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
	  if (vi >= v.length() || (v[vi++] != c))
	    return;
      }
    }
  }

  if (vi < v.length())
    return;

  if (date)
    *date = WDate(dateParse.year, dateParse.month, dateParse.day);

  if (time) {
    if (timeParse.parseAMPM && timeParse.haveAMPM) {
      if (timeParse.pm)
	timeParse.hour = (timeParse.hour % 12) + 12;
      else
	timeParse.hour = timeParse.hour % 12;
    }

    *time = WTime(timeParse.hour, timeParse.minute, timeParse.sec,
		  timeParse.msec);
  }
}

WString WDateTime::toString() const
{
  return toString(defaultFormat());
}

WString WDateTime::toString(const WString& format, bool localized) const
{
  WDate d = date();
  WTime t = time();

  return toString(&d, &t, format, localized, 0);
}

WString WDateTime::toString(const WDate *date, const WTime *time,
                            const WString& format, bool localized,
			    int zoneOffset)
{
  if ((date && !date->isValid()) || (time && !time->isValid())) {
    if (WApplication::instance()) {
        return tr("Wt.WDateTime.null");
    } else {
        return WString::fromUTF8("Null");
    }
  }

  WStringStream result;
  std::string f = format.toUTF8() + std::string(3, 0);

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  /*
   * We need to scan the format first to determine whether it contains
   * 'A(P)' or 'a(p)'
   */
  bool useAmPm = false;
  if (time)
    useAmPm = WTime::usesAmPm(format);
 
  for (unsigned i = 0; i < f.length() - 3; ++i) {
    if (inQuote) {
      if (f[i] != '\'') {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else
          result << f[i];
      } else {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
          result << f[i];
	} else
	  gotQuoteInQuote = true;
      }
    }

    if (!inQuote) {
      bool handled = false;
      if (date)
	handled = date->writeSpecial(f, i, result, localized);
      if (!handled && time)
        handled = time->writeSpecial(f, i, result, useAmPm, zoneOffset);

      if (!handled) {
	if (f[i] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
          result << f[i];
      }
    }
  }
  return WString::fromUTF8(result.str());
}

WDateTime WDateTime::fromTime_t(std::time_t t) {
  WDateTime dt;
  dt.setTime_t(t);
  
  return dt;
}

WDateTime WDateTime::fromTimePoint(const std::chrono::system_clock::time_point& t) {
  return WDateTime(t);
}

}

#endif // DOXYGEN_ONLY
