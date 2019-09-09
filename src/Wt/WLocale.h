// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLOCALE_H_
#define WLOCALE_H_

#include <Wt/WGlobal.h>
#include <Wt/WString.h>

namespace date {
  class time_zone;
}

namespace Wt {

/*! \class WLocale Wt/WLocale.h Wt/WLocale.h
 *  \brief A locale
 *
 * This class provides localization support for an application.
 *
 * It can be used to determine the formatting of dates and numbers,
 * and by WLocalDateTime to determine the time zone.
 *
 * Note that only the name() parameter is automatically retrieved
 * from the browser, if possible, and message resources are deduced
 * based on that. All other parameters have to be configured.
 *
 * \sa WApplication::locale()
 */
class WT_API WLocale
{
public:
  /*! \brief Default constructor.
   *
   * Configures a locale with an empty name, and US conventions:
   *  - "yyyy/MM/dd" format for dates.
   *  - "." as decimal point, and no group separator.
   */
  WLocale();

  /*! \brief Copy constructor.
   */
  WLocale(const Wt::WLocale& locale);

  /*! \brief Creates a locale by name.
   *
   * The locale name is a string such as "en" (for English) or "en_GB"
   * (for UK English).
   */
  WLocale(const std::string& locale);

  /*! \brief Creates a locale by name.
   *
   * The locale name is a string such as "en" (for English) or "en_GB"
   * (for UK English).
   */
  WLocale(const char *locale);

  /*! \brief Assignment operator.
   */
  WLocale& operator=(const WLocale& locale);

  /*! \brief Sets the decimal point.
   *
   * Sets the character used to separate integral from fractional
   * digits in a double.
   *
   * \note the argument is a UTF-8 encoded character and can thus be up
   *       to 4 byte.
   */
  void setDecimalPoint(WT_UCHAR c);

  /*! \brief Returns the decimal point.
   *
   * \sa setDecimalPoint()
   */
  WT_UCHAR decimalPoint() const { return decimalPoint_; }

  /*! \brief Sets the decimal group separator.
   *
   * Sets the character used to separate thousands in a number.
   *
   * \note the argument is a UTF-8 encoded character and can thus be up
   *       to 4 byte.
   */
  void setGroupSeparator(WT_UCHAR c);

  /*! \brief Returns the decimal group separator.
   *
   * \sa setGroupSeparator()
   */
  WT_UCHAR groupSeparator() const { return groupSeparator_; }

  /*! \brief Sets the time zone.
   *
   * This sets the time zone (used by the client). This
   * is a time zone from Howard Hinnant's date library,
   * which will become integrated into the C++20 standard.
   *
   * A time zone can be retrieved with \p date::locate_zone using
   * the name of the time zone. The time zone name may be retrieved with
   * WEnvironment::timeZoneName(), if supported by the browser.
   *
   * WEnvironment::timeZoneOffset() will also (if available) contain
   * an offset. This offset could be used to let the user select a
   * time zone from a list of timezones. You can get a list of
   * zones with \p date::get_tzdb().zones.
   *
   * The timezone is used by WLocalDateTime.
   *
   * \sa https://howardhinnant.github.io/date/date.html
   * \sa https://howardhinnant.github.io/date/tz.html
   * \sa WEnvironment::timeZoneName()
   * \sa WEnvironment::timeZoneOffset()
   */
  void setTimeZone(const date::time_zone *zone);

  /*! \brief Returns the user's time zone.
   *
   * \sa setTimeZone()
   */
  const date::time_zone *timeZone() const { return timeZone_; }

  /*! \brief Sets the date format.
   *
   * Sets the default format for date entry, e.g. as used in
   * WDateValidator. See WDate::toString() for the supported syntax.
   *
   * The default date format is "yyyy-MM-dd".
   */
  void setDateFormat(const WT_USTRING& format);

  /*! \brief Returns the date format.
   *
   * Returns the date format.
   */
  WT_USTRING dateFormat() const { return dateFormat_; }

  /*! \brief Sets the time format.
   *
   *  Sets the default format for time entry, eg. as used in
   *  WTimeValidator. See WTime::toString() for the supported syntax.
   *
   *  The default time format is "HH:mm:ss".
   */
  void setTimeFormat(const WT_USTRING &format);

  /*! \brief Returns the time format.
   *
   * Returns the time format.
   */
  WT_USTRING timeFormat() const { return timeFormat_; }

  /*! \brief Sets the date/time format.
   *
   * Sets the format for a localized time stamp (using
   * WLocalDateTime::toString()). See WDateTime::toString() for the
   * supported syntax.
   *
   * The default date/time format is "yyyy-MM-dd HH:mm:ss".
   */
  void setDateTimeFormat(const WT_USTRING& format);

  /*! \brief Returns the date/time format.
   *
   * Returns the date/time format.
   */
  WT_USTRING dateTimeFormat() const { return dateTimeFormat_; }

  /*! \brief Returns the locale name.
   *
   * This is the name of the locale that was set through the
   * constructor.
   */
  std::string name() const { return name_; }

  /*! \brief Returns the current (user) locale.
   *
   * This returns WApplication::instance()->locale() if the
   * WApplication::instance() != 0, or a default locale otherwise.
   */
  static const WLocale& currentLocale();

  /*! \brief Parses a floating point number.
   *
   * Throws a runtime exception if the number could not be parsed.
   */
  double toDouble(const WT_USTRING& value) const;

  /*! \brief Parses an integer number.
   *
   * Throws a runtime exception if the number could not be parsed.
   */
  int toInt(const WT_USTRING& value) const;

  /*! \brief Formats an integer number.
   */
  WT_USTRING toString(int value) const;

  /*! \brief Formats an integer number.
   */
  WT_USTRING toString(unsigned int value) const;

  /*! \brief Formats an integer number.
   */
  WT_USTRING toString(long value) const;

  /*! \brief Formats an integer number.
   */
  WT_USTRING toString(unsigned long value) const;

  /*! \brief Formats an integer number.
   */
  WT_USTRING toString(long long value) const;

  /*! \brief Formats an integer number.
   */
  WT_USTRING toString(unsigned long long value) const;

  /*! \brief Formats a floating point number.
   */
  WT_USTRING toString(double value) const;

  /*! \brief Formats a floating point number with given precision.
   */
  WT_USTRING toFixedString(double value, int precision) const;

  //boost::local_time::time_zone_ptr time_zone_ptr() const;

  /*! \brief Sets the current (user) locale.
   *
   * This calls WApplication::setLocale().
   *
   * When there is no session (e.i. WApplication::instance() == 0),
   * for example in WResource::handleRequest(), then this sets the locale
   * for the current request.
   */
  static void setCurrentLocale(const WLocale &locale);

private:
  std::string name_;
  WT_UCHAR decimalPoint_, groupSeparator_;
  WT_USTRING dateFormat_, timeFormat_, dateTimeFormat_;

  const date::time_zone *timeZone_;

  bool isDefaultNumberLocale() const;

  WT_USTRING integerToString(const std::string& v) const;
  WT_USTRING doubleToString(std::string v) const;
  std::string addGrouping(const std::string& v, unsigned decimalPoint) const;
};

}

#endif // WLOCALE_H_
