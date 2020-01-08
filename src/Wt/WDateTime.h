// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDATE_TIME_H_
#define WDATE_TIME_H_

#include <Wt/WDllDefs.h>
#include <Wt/WException.h>
#include <Wt/WLocale.h>
#include <Wt/WString.h>
#include <exception>
#include <chrono>

namespace Wt {

/*! \class WDateTime Wt/WDateTime.h Wt/WDateTime.h
 *  \brief A calendar date and clock time.
 *
 * The date time class combines the functionality of a WDate (for a
 * calendar date) and WTime (for clock time) into a single class.
 *
 * This class stores and represents the date time in UTC. To deal with local
 * (wall clock) time, see WLocalDateTime. To convert from UTC to local time
 * use toLocalTime().
 *
 * <h3>i18n</h3>
 *
 * The strings returned by toString() and timeTo() can be localized.
 * If the thread using a WDateTime is bound to a WApplication (i.e.
 * WApplication::instance() does not return 0), the strings
 * can be localized by overriding the default values for
 * the localized string keys in the resource bundles of the WApplication.
 * If the thread using a WDateTime is not bound to a WApplication (i.e.
 * WApplication::instance() returns 0), English strings will be used.
 *
 * For toString(), localization is handled through the i18n functionality
 * of the WDate class. An invalid WDateTime is converted to a string
 * as "Null":
 * - Wt.WDateTime.null: Null
 *
 * The timeTo() method contains the following localized strings:
 * - Wt.WDateTime.seconds
 * - Wt.WDateTime.minutes
 * - and so on
 *
 * The default translations of Wt.WDateTime.seconds are "one second" and "{1} seconds".
 * The WString::trn() function is used for the translation, so different strings
 * can be provided for different plural forms.
 * The placeholder {1} will be replaced by the actual number of seconds. The
 * same keys also exist for minutes, hours, days, weeks, months and years.
 */
class WT_API WDateTime
{
public:
  /*! \brief Creates a <i>Null</i> datetime.
   *
   * A time for which isNull() returns \c true. A <i>Null</i> datetime
   * is also invalid.
   *
   * \sa isValid(), isNull()
   */
  WDateTime();

  /*! \brief Creates a datetime given a date.
   *
   * The time is set to midnight (00:00). The datetime is valid if the
   * \p date is valid.
   */
  explicit WDateTime(const WDate& date);

  /*! \brief Creates a datetime given a date and time.
   *
   * The datetime is valid if both \p date and \p time are valid.
   */
  WDateTime(const WDate& date, const WTime& time);

  /*! \brief Creates a date time.
   */
  explicit WDateTime(const std::chrono::system_clock::time_point timepoint);

  /*! \brief Sets the time in seconds from the Epoch.
   *
   * The \p time is the number of seconds since the Epoch (00:00:00
   * UTC, January 1, 1970).
   *
   * \sa toTime_t()
   */
  void setTime_t(std::time_t time);

  /*! \brief Sets the date time.
   */
  void setTimePoint(const std::chrono::system_clock::time_point& timepoint);

  /*! \brief Adds milliseconds.
   *
   * Returns a datetime that is \p ms milliseconds later than this
   * datetime. Negative values for \p ms will result in a datetime that
   * is as many milliseconds earlier.
   *
   * Returns a null date if the current date time is invalid or the new
   * date time is out of range.
   */
  WDateTime addMSecs(int ms) const;

  /*! \brief Adds seconds.
   *
   * Returns a datetime that is \p s seconds later than this
   * datetime. Negative values for \p s will result in a datetime that
   * is as many seconds earlier.
   *
   * Returns a null date if the current date time is invalid or the new
   * date time is out of range.
   */
  WDateTime addSecs(int s) const;

  /*! \brief Adds days.
   *
   * Returns a datetime that is \p ndays later than this
   * datetime. Negative values for \p ndays will result in a datetime
   * that is as many days earlier.
   *
   * Returns a null date if the current date time is invalid or the new
   * date time is out of range.
   *
   * \sa addMonths(), addYears()
   */
  WDateTime addDays(int ndays) const;

  /*! \brief Adds months.
   *
   * Returns a datetime that is the same day of the month, but \p
   * nmonths later than this date. Negative values for \p nmonths will
   * result in a datetime that is as many months earlier.
   *
   * Returns a null date if the current date time is invalid or the new
   * date time is out of range.
   *
   * \sa addDays(), addYears()
   */
  WDateTime addMonths(int nmonths) const;

  /*! \brief Adds years.
   *
   * Returns a datetime that is \p nyears later than this
   * datetime. Negative values for \p nyears will result in a datetime
   * that is as many years earlier.
   *
   * Returns a null date if the current date time is invalid or the new
   * date time is out of range.
   *
   * \sa addDays(), addMonths()
   */
  WDateTime addYears(int nyears) const;

  /*! \brief Returns if this datetime is Null.
   *
   * A <i>null</i> time is also invalid.
   *
   * \sa isValid(), WDateTime()
   */
  bool isNull() const;

  /*! \brief Returns if this datetime is valid.
   *
   * A date time is only valid if its date and time parts are valid.
   */
  bool isValid() const;

  /*! \brief Sets the date part.
   *
   * Changes the date part part, leaving the time unmodified. If no time
   * was set, it is set to 00:00.
   *
   * \sa setTime()
   */
  void setDate(const WDate& date);

  /*! \brief Returns the date part.
   *
   * Returns the date part.
   *
   * \sa time()
   */
  const WDate date() const;

  /*! \brief Sets the time part.
   *
   * If no valid date is set, the time is not set either.
   *
   * \sa setDate()
   */
  void setTime(const WTime& time);

  /*! \brief Returns the time part.
   *
   * \sa setTime()
   */
  const WTime time() const;

  /*! \brief Returns the number of seconds since the Epoch.
   *
   * This returns the number of seconds since the Epoch (00:00:00 UTC,
   * January 1, 1970) represented by this datetime.
   */
  std::time_t toTime_t() const;

  /*! \brief Returns the boost time.
   *
   * \sa fromPosixTime()
   */
  std::chrono::system_clock::time_point toTimePoint() const;

  /*! \brief Converts to a local time.
   *
   * The conversion is based on the fact that WDateTime represents UTC time.
   * 
   * This is the reverse of WLocalDateTime::toUTC()
   */
  WLocalDateTime toLocalTime(const WLocale& locale = WLocale::currentLocale())
    const;

  /*! \brief Returns the difference between two datetime values (in seconds).
   *
   * The result is negative if other is earlier than this.
   *
   * Returns 0 if either date is invalid.
   *
   * \sa daysTo()
   */
  int secsTo(const WDateTime& other) const;

  /*! \brief Returns the difference between two datetime values (in days).
   *
   * The result is negative if other is earlier than this.
   *
   * Returns 0 if either date is invalid.
   *
   * \sa secsTo()
   */
  int daysTo(const WDateTime& other) const;

  /*! \brief Returns the difference between two datetime values (as text).
   *
   * This returns a textual representation of the approximate
   * difference between this time and \p other. The textual
   * representation returns the difference as a number of seconds,
   * minutes, hours, days, weeks, months, or years, using the coarsest
   * unit that is more than \p minValue.
   *
   * Returns an empty string if either date is invalid.
   *
   * \sa daysTo(), secsTo()
   */
  WString timeTo(const WDateTime& other, std::chrono::seconds minValue = std::chrono::seconds(1)) const;

  /*! \brief Compares two datetime values.
   */
  bool operator< (const WDateTime& other) const;

  /*! \brief Compares two datetime values.
   */
  bool operator<= (const WDateTime& other) const;

  /*! \brief Compares two datetime values.
   */
  bool operator> (const WDateTime& other) const;

  /*! \brief Compares two datetime values.
   */
  bool operator>= (const WDateTime& other) const;

  /*! \brief Compares two datetime values.
   */
  bool operator== (const WDateTime& other) const;

  /*! \brief Compares two datetime values.
   */
  bool operator!= (const WDateTime& other) const;

  static WT_USTRING defaultFormat();

  /*! \brief Formats this datetime to a string using a default format.
   *
   * The default format is "ddd MMM d hh:mm:ss yyyy".
   */
  WT_USTRING toString() const;

  /*! \brief Formats this time to a string using a specified format.
   *
   * The \p format is a string which mixes the format for WDate and
   * WTime.
   *
   * \sa WDate::toString(), WTime::toString()
   */
  WT_USTRING toString(const WT_USTRING& format, bool localized = true) const;

  /*! \brief Parses a string to a time using a default format.
   *
   * The default format is "ddd MMM d hh:mm:ss yyyy".
   *
   * \sa WDate::fromString(), WTime::fromString().
   */
  static WDateTime fromString(const WT_USTRING& s);

  /*! \brief Parses a string to a time using a specified format.
   *
   * The \p format is a string which mixes the format for WDate and
   * WTime.
   *
   * \sa WDate::fromString(), WTime::toString().
   */
  static WDateTime fromString(const WT_USTRING& s, const WT_USTRING& format);

  /*! \brief Reports the current datetime (UTC clock).
   *
   * This method returns the datetime as indicated by the system clock
   * of the server, in UTC.
   */
  static WDateTime currentDateTime();

  /*! \brief Creates a date time based on a number of seconds since the Epoch.
   *
   * \sa setTime_t()
   */
  static WDateTime fromTime_t(std::time_t seconds);

  /*! \brief Creates a date time from boost's date time type.
   *
   * \sa toPosixTime()
   */
  static WDateTime fromTimePoint(const std::chrono::system_clock::time_point& datetime);

private:
  std::chrono::system_clock::time_point datetime_;
  bool invalid_date_time_ = true;
  bool null_ = true;

  enum class CharState { CharUnhandled, CharHandled, CharInvalid };

  static void fromString(WDate *date, WTime *time, const WString& s,
			 const WString& format);
  static WString toString(const WDate *date, const WTime *time,
                          const WString& format, bool localized,
			  int zoneOffset);

  friend class WDate;
  friend class WTime;
  friend class WLocalDateTime;
};

}

#endif // WDATE_TIME_H_
