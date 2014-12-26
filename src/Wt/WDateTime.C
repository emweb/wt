/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdlib.h>

#include "Wt/WApplication"
#include "Wt/WDateTime"
#include "Wt/WDate"
#include "Wt/WLocalDateTime"
#include "Wt/WTime"

#ifndef DOXYGEN_ONLY

namespace posix = boost::posix_time;
namespace gregorian = boost::gregorian;

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
  }

InvalidDateTimeException::InvalidDateTimeException()
  : WException("Error: Attempted operation on an invalid WDateTime")
{ }

WDateTime::WDateTime()
{ }

WDateTime::WDateTime(const WDate& date)
{
  if (date.isValid()) {
    gregorian::date d = date.toGregorianDate();
    posix::time_duration t(0, 0, 0, 0);

    datetime_ = posix::ptime(d, t);
  } else
    datetime_ = posix::ptime(posix::neg_infin);
}

WDateTime::WDateTime(const WDate& date, const WTime& time)
{
  if (date.isValid() && time.isValid()) {
    gregorian::date d = date.toGregorianDate();
    posix::time_duration t = time.toTimeDuration();

    datetime_ = posix::ptime(d, t);
  } else
    datetime_ = posix::ptime(posix::neg_infin);
}

WDateTime::WDateTime(posix::ptime dt)
  : datetime_(dt)
{ }

void WDateTime::setTime_t(std::time_t t)
{
  datetime_ = posix::from_time_t(t);
}

void WDateTime::setPosixTime(const posix::ptime& dt)
{
  datetime_ = dt;
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
    gregorian::date d = datetime_.date();
    return WDate(d);
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
  if (isValid()) {
    posix::time_duration d = datetime_.time_of_day();
    posix::time_duration::fractional_seconds_type ticks_per_msec =
      posix::time_duration::ticks_per_second() / 1000;
    posix::time_duration::fractional_seconds_type msec =
      d.fractional_seconds();
    msec = msec / ticks_per_msec;
    return WTime(d.hours(), d.minutes(), d.seconds(), (int)msec);
  } else
    return WTime();
}

WDateTime WDateTime::addMSecs(int ms) const
{
  if (isValid()) {
    posix::time_duration::fractional_seconds_type ticks_per_msec =
      posix::time_duration::ticks_per_second() / 1000;
    posix::ptime dt = datetime_ + posix::time_duration(0, 0, 0,
						       ms * ticks_per_msec);
    if (dt.is_not_a_date_time())
      dt = posix::ptime(posix::neg_infin);

    return WDateTime(dt);
  } else
    return WDateTime();
}

WDateTime WDateTime::addSecs(int s) const
{
  if (isValid()) {
    posix::ptime dt = datetime_ + posix::time_duration(0, 0, s, 0);
    if (dt.is_not_a_date_time())
      dt = posix::ptime(posix::neg_infin);
    return WDateTime(dt);
  } else
    return WDateTime();
}

WDateTime WDateTime::addDays(int ndays) const
{
  if (isValid()) {
    posix::ptime dt = datetime_ + gregorian::days(ndays);
    if (dt.is_not_a_date_time())
      dt = posix::ptime(posix::neg_infin);
    return WDateTime(dt);
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
  return datetime_.is_not_a_date_time();
}

bool WDateTime::isValid() const
{
  return !datetime_.is_special();
}

std::time_t WDateTime::toTime_t() const
{
  if (isValid())
    return (datetime_ - posix::ptime(gregorian::date(1970, 1, 1)))
      .total_seconds();
  else
    return (std::time_t) 0;
}

posix::ptime WDateTime::toPosixTime() const
{
  return datetime_;
}

WLocalDateTime WDateTime::toLocalTime(const WLocale& locale) const
{
  return WLocalDateTime
    (boost::local_time::local_date_time(datetime_, locale.time_zone_ptr()),
     locale.dateTimeFormat());
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

WString WDateTime::timeTo(const WDateTime& other, int minValue) const
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
  else if (abs(secs) < 60 * minValue)
    if (WApplication::instance()) {
      return secs > 1 ? tr("Wt.WDateTime.seconds").arg(secs) :
        tr("Wt.WDateTime.second");
    } else {
      return boost::lexical_cast<std::string>(secs) + " second"
        + multiple(secs, "s");
    }
  else {
    int minutes = secs / 60;
    if (abs(minutes) < 60 * minValue)
      if (WApplication::instance()) {
        return minutes > 1 ? tr("Wt.WDateTime.minutes").arg(minutes) :
          tr("Wt.WDateTime.minute");
      } else {
        return boost::lexical_cast<std::string>(minutes) + " minute"
          + multiple(minutes, "s");
      }
    else {
      int hours = minutes / 60;
      if (abs(hours) < 24 * minValue)
        if (WApplication::instance()) {
          return hours > 1 ? tr("Wt.WDateTime.hours").arg(hours) :
            tr("Wt.WDateTime.hour");
        } else {
          return boost::lexical_cast<std::string>(hours) + " hour"
            + multiple(hours, "s");
        }
      else {
	int days = hours / 24;
        if (abs(days) < 7 * minValue)
          if (WApplication::instance()) {
            return days > 1 ? tr("Wt.WDateTime.days").arg(days) :
              tr("Wt.WDateTime.day");
          } else {
            return boost::lexical_cast<std::string>(days) + " day"
              + multiple(days, "s");
          }
	else {
	  if (abs(days) < 31 * minValue) {
	    int weeks = days / 7;
            if (WApplication::instance()) {
              return weeks > 1 ? tr("Wt.WDateTime.weeks").arg(weeks) :
                tr("Wt.WDateTime.week");
            } else {
              return boost::lexical_cast<std::string>(weeks) + " week"
                + multiple(weeks, "s");
            }
	  } else {
	    if (abs(days) < 365 * minValue) {
	      int months = days / 30;
              if (WApplication::instance()) {
                return months > 1 ? tr("Wt.WDateTime.months").arg(months) :
                  tr("Wt.WDateTime.month");
              } else {
                return boost::lexical_cast<std::string>(months) + " month"
                  + multiple(months, "s");
              }
	    } else {
	      int years = days / 365;
              if (WApplication::instance()) {
                return years > 1 ? tr("Wt.WDateTime.years").arg(years) :
                  tr("Wt.WDateTime.year");
              } else {
                return years > 1 ? tr("Wt.WDateTime.years").arg(years) :
                  tr("Wt.WDateTime.year");
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
  return WDateTime(posix::microsec_clock::universal_time());
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
	} else
	  gotQuoteInQuote = true;
      }
    }

    if (!inQuote) {
      CharState state = CharUnhandled;

      if (date) {
	CharState dateState = WDate::handleSpecial(c, v, vi, dateParse, format);
	if (dateState == CharInvalid)
	  return;
	else if (dateState == CharHandled)
	  state = CharHandled;
      }

      if (time) {
	CharState timeState = WTime::handleSpecial(c, v, vi, timeParse, format);
	if (timeState == CharInvalid)
	  return;
	else if (timeState == CharHandled)
	  state = CharHandled;
      }

      if (!finished && state == CharUnhandled) {
	if (c == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
	  if (vi >= v.length() || (v[vi++] != c))
	    return;
      }
    }
  }

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

  std::stringstream result;
  std::string f = format.toUTF8() + std::string(3, 0);

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  /*
   * We need to scan the format first to determine whether it contains
   * 'A(P)' or 'a(p)'
   */
  bool useAMPM = false;

  if (time) {
    for (unsigned i = 0; i < f.length() - 3; ++i) {
      if (inQuote) {
	if (f[i] != '\'') {
	  if (gotQuoteInQuote) {
	    gotQuoteInQuote = false;
	    inQuote = false;
	  }
	} else {
	  if (gotQuoteInQuote)
	    gotQuoteInQuote = false;
	  else
	    gotQuoteInQuote = true;
	}
      }

      if (!inQuote) {
	if (f[i] == 'a' || f[i] == 'A') {
	  useAMPM = true;
	  break;
	} else if (f[i] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	}
      }
    }
  }
 
  for (unsigned i = 0; i < f.length() - 3; ++i) {
    if (inQuote) {
      if (f[i] != '\'') {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else
	  result.put(f[i]);
      } else {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  result.put(f[i]);
	} else
	  gotQuoteInQuote = true;
      }
    }

    if (!inQuote) {
      bool handled = false;
      if (date)
	handled = date->writeSpecial(f, i, result, localized);
      if (!handled && time)
        handled = time->writeSpecial(f, i, result, useAMPM, zoneOffset);

      if (!handled) {
	if (f[i] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
	  result.put(f[i]);
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

WDateTime WDateTime::fromPosixTime(const posix::ptime& t) {
  return WDateTime(t);
}

}

#endif // DOXYGEN_ONLY
