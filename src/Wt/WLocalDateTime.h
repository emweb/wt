// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLOCAL_DATE_TIME_H_
#define WLOCAL_DATE_TIME_H_

#include <Wt/WDllDefs.h>
#include <Wt/WException.h>
#include <Wt/WLocale.h>
#include <Wt/WString.h>
#include <exception>
#include <chrono>

namespace Wt {

/*! \class WLocalDateTime Wt/WLocalDateTime.h Wt/WLocalDateTime.h
 *  \brief A localized calendar date and clock time.
 *
 * This presents a localized date time, which represents a date time
 * in a given locale.
 *
 * It is recommended to only use this to display timestamps to the
 * user, and use WDateTime (which stores UTC time) throughout your
 * business logic. For this reason, this class does not contain any
 * methods to calculate with time.
 *
 * \sa WDateTime::toLocalTime()
 */
class WT_API WLocalDateTime
{
public:
  /*! \brief Creates a <i>Null</i> datetime.
   *
   * A time for which isNull() returns \c true. A <i>Null</i> datetime
   * is also invalid.
   *
   * \sa isValid(), isNull()
   */
  WLocalDateTime(const WLocale& locale = WLocale::currentLocale());

  /*! \brief Creates a datetime given a date and time.
   *
   * The datetime is valid if both \p date and \p time are valid and the
   * given time could correctly be interpreted in the local time zone:
   * - the time must exist (on the day that the time changes from 2AM to
   *   3AM the time 2.30AM does not exist)
   * - the time must be unambiguous. On the day that time changes from
   *   3AM to 2AM, the time 2.30AM is not unambiguous. In that case the
   *   ambiguity must be resolved by using setDateTime() indicating whether
   *   the time is day light savings time or not.
   */
  WLocalDateTime(const WDate& date, const WTime& time,
		 const WLocale& locale = WLocale::currentLocale());

  /*! \brief Returns if this datetime is <i>Null</i>.
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

  /*! \brief Sets the local date and time.
   *
   * The datetime is valid if both \p date and \p time are valid and the
   * given time could correctly be interpreted in the local time zone:
   * - the time exists (on the day that the time changes from 2AM to
   *   3AM the time 2.30AM does not exist)
   * - the time is unambiguous (on the day that time changes from 3AM to 2AM.
   *   the time 2.30AM is not unambigous). In the latter case,
   *   setDateTime(const WDate&, const WTime&, bool) may still be used.
   */
  void setDateTime(const WDate& date, const WTime& time);

  /*! \brief Sets the local date and time, indicating day-light savings.
   *
   * The datetime is valid if both \p date and \p time are valid and the
   * given time could correctly be interpreted in the local time zone with
   * the indicated day light savings indication.
   */
  void setDateTime(const WDate& date, const WTime& time, bool dst);

  /*! \brief Sets the date part.
   *
   * Changes the date part part, leaving the time unmodified. If no
   * time was set, it is set to 00:00.
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
  WDate date() const;

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
  WTime time() const;

  /*! \brief Returns the time zone offset.
   *
   * This returns the time zone offset to UTC in minutes. A positive value
   * thus means that the local time is ahead of UTC.
   */
  int timeZoneOffset() const;

  /*! \brief Returns the time zone.
   *
   * \sa WLocale::setTimeZone()
   */
  const date::time_zone *timeZone() const;

  /*! \brief Converts to UTC.
   *
   * This is the reverse of WDateTime::toLocalTime()
   */
  WDateTime toUTC() const;

  /*! \brief Formats this datetime to a string using the locale format.
   *
   * \sa WLocale::dateTimeFormat()
   */
  WT_USTRING toString() const;

  /*! \brief Formats this datetime to a string in a custom format.
   *
   * \sa WLocale::dateTimeFormat()
   */
  WT_USTRING toString(const WT_USTRING& format) const;

  /*! \brief Parses a string to a time using the locale format.
   *
   * \sa WLocale::dateTimeFormat()
   */
  static WLocalDateTime fromString(const WT_USTRING& s,
				   const WLocale& locale = WLocale::currentLocale());

  /*! \brief Reports the current datetime.
   *
   * This method returns the current datetime in the given
   * locale.
   *
   * If no time zone has been configured for the given locale
   * (locale.timeZone() is empty), then the
   * offset is based on WEnvironment::timeZoneOffset().
   */
  static WLocalDateTime currentDateTime(const WLocale& locale = WLocale::currentLocale());

  /*! \brief Reports the current local server time.
   */
  static WLocalDateTime currentServerDateTime();

  /*! \brief Compares two values.
   */
  bool operator< (const WLocalDateTime& other) const;

  /*! \brief Compares two datetime values.
   */
  bool operator== (const WLocalDateTime& other) const;

  /*! \brief Compares two datetime values.
   */
  bool operator!= (const WLocalDateTime& other) const;

  // Internal: for tests
  static WLocalDateTime offsetDateTime(const std::chrono::system_clock::time_point& dt,
                                       std::chrono::minutes offset, const WT_USTRING& format);

private:
  class OffsetZone;

  std::chrono::system_clock::time_point datetime_; // UTC clock
  WT_USTRING format_;
  const date::time_zone *zone_;
  std::shared_ptr<OffsetZone> customZone_;
  bool valid_, null_;

  WLocalDateTime(const std::chrono::system_clock::time_point& dt,
                 const date::time_zone *zone, const WT_USTRING& format);
  WLocalDateTime(const std::chrono::system_clock::time_point& dt,
                 const std::shared_ptr<OffsetZone>& zone, const WT_USTRING& format);

  friend class WDateTime;

  void setInvalid();

  static WLocalDateTime currentTime(std::chrono::minutes offset, const WT_USTRING& format);

};

}

#endif // WLOCAL_DATE_TIME_H_
