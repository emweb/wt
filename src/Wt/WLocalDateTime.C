/*
 * Copyright (C) 2013 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WStringStream"
#include "Wt/WLocalDateTime"
#include "Wt/WLogger"
#include "Wt/WDateTime"
#include "Wt/WDate"
#include "Wt/WTime"

namespace Wt {

LOGGER("WDateTime");

WLocalDateTime::WLocalDateTime(const boost::local_time::local_date_time& dt,
			       const WT_USTRING& format)
  : datetime_(dt),
    format_(format)
{ }

WLocalDateTime::WLocalDateTime(const WLocale& locale)
  : datetime_(boost::posix_time::ptime(), locale.time_zone_ptr()),
    format_(locale.dateTimeFormat())
{ }

/*
 * Todo, add overload which indicates DST
 */
WLocalDateTime::WLocalDateTime(const WDate& date, const WTime& time,
			       const WLocale& locale)
  : datetime_(boost::posix_time::ptime(), locale.time_zone_ptr()),
    format_(locale.dateTimeFormat())
{ 
  setDateTime(date, time);
}

bool WLocalDateTime::isNull() const
{
  return datetime_.is_not_a_date_time();
}

bool WLocalDateTime::isValid() const
{
  return !datetime_.is_special();
}

void WLocalDateTime::setDateTime(const WDate& date, const WTime& time)
{
  if (date.isValid() && time.isValid()) {
    datetime_ = boost::local_time::local_date_time
      (date.toGregorianDate(), time.toTimeDuration(),
       datetime_.zone(),
       boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);

    if (datetime_.is_not_a_date_time()) {
      LOG_WARN("Invalid local date time ("
	       << date.toString() << " "
	       << time.toString() << ") in zone "
	       << (datetime_.zone() ?
		   datetime_.zone()->to_posix_string() :
		   "<no zone>"));

      setInvalid();
    }
  } else
    setInvalid();
}

void WLocalDateTime::setInvalid()
{
  datetime_
    = boost::local_time::local_date_time
	(boost::posix_time::ptime(boost::posix_time::neg_infin),
	 datetime_.zone());
}

void WLocalDateTime::setDateTime(const WDate& date, const WTime& time,
				 bool dst)
{
  if (date.isValid() && time.isValid()) {
    try {
      datetime_ = boost::local_time::local_date_time
	(date.toGregorianDate(), time.toTimeDuration(),
	 datetime_.zone(), dst);
      if (datetime_.is_not_a_date_time()) {
	LOG_WARN("Invalid local date time ("
		 << date.toString() << " "
		 << time.toString() << " "
		 << "dst=" << dst << ") in zone "
		 << (datetime_.zone() ?
		     datetime_.zone()->to_posix_string() :
		     "<no zone>"));
	setInvalid();
      }
    } catch (std::exception& e) {
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
  if (isValid()) {
    boost::gregorian::date d = datetime_.local_time().date();
    return WDate(d);
  } else
    return WDate();
}

void WLocalDateTime::setTime(const WTime& time)
{
  if (isValid())
    setDateTime(date(), time);
}

WTime WLocalDateTime::time() const
{
  if (isValid()) {
    boost::posix_time::time_duration d = datetime_.local_time().time_of_day();
    boost::posix_time::time_duration::fractional_seconds_type ticks_per_msec =
      boost::posix_time::time_duration::ticks_per_second() / 1000;
    boost::posix_time::time_duration::fractional_seconds_type msec =
      d.fractional_seconds();
    msec = msec / ticks_per_msec;
    return WTime(d.hours(), d.minutes(), d.seconds(), (int)msec);
  } else
    return WTime();
}

WDateTime WLocalDateTime::toUTC() const
{
  if (isValid())
    return WDateTime(datetime_.utc_time());
  else
    return WDateTime();
}

WT_USTRING WLocalDateTime::toString() const
{
  return toString(format_);
}

int WLocalDateTime::timeZoneOffset() const
{
  return (datetime_.local_time() - datetime_.utc_time())
    .total_seconds() / 60;
}

std::string WLocalDateTime::timeZone() const
{
  if (datetime_.zone())
    return datetime_.zone()->to_posix_string();
  else
    return std::string();
}

WT_USTRING WLocalDateTime::toString(const WT_USTRING& format) const
{
  WDateTime dt(datetime_.local_time());
  WDate d = dt.date();
  WTime t = dt.time();

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

  if (locale.timeZone().empty() && app)
    return currentTime(app->environment().timeZoneOffset());
  else
    return WDateTime::currentDateTime().toLocalTime(locale);
}

WLocalDateTime WLocalDateTime::currentTime(int offset)
{
  /*
   * Fabricate a time zone that reflects the time zone offset.
   * This is entirely accurate for the 'current' date time.
   */
  WStringStream tz;
  tz << "LOC";
  if (offset < 0) {
    offset = -offset;
    tz << '-';
  }

  WTime t(0, 0, 0);
  t = t.addSecs(offset * 60);
  tz << t.hour();
  if (t.minute() != 0)
    tz << ':' << t.minute();
    
  WLocale tmpLocale;
  tmpLocale.setTimeZone(tz.str());
  return WDateTime::currentDateTime().toLocalTime(tmpLocale);
}

WLocalDateTime WLocalDateTime::currentServerDateTime()
{
  boost::posix_time::ptime utcNow
    = boost::posix_time::second_clock::universal_time();
  boost::posix_time::ptime localNow
    = boost::posix_time::second_clock::local_time();

  int offset = (localNow - utcNow).total_seconds() / 60;

  return currentTime(offset);
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
