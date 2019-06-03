// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDATE_H_
#define WDATE_H_

#include <Wt/WDateTime.h>
#include <Wt/WException.h>
#include <Wt/WString.h>
#include <Wt/WStringStream.h>
#include <exception>
#include <chrono>

namespace Wt {

/*! \class WDate Wt/WDate.h Wt/WDate.h
 *  \brief A value class that represents a date on the Gregorian calendar.
 *
 * Class which holds a date on the gregorian calendar, specified as
 * day/month/year.
 *
 * A %WDate may either specify a valid date, or be a <i>Null</i> date
 * (using the default constructor WDate::WDate(), for which isNull()
 * returns \c true).
 *
 * A valid date may be specified by year, month, and day of month
 * (using the \link WDate::WDate(int, int, int)
 * WDate(year, month, day) \endlink constructor,
 * or the setDate() method). When attempting to specify an
 * invalid date (with an impossible combination of year/month/date),
 * isIvalid() will return \c false.
 *
 * The class provides a flexible way for converting between strings
 * and dates. Use toString() to convert to strings, and fromString()
 * for parsing strings. Both methods take a format string, and the
 * same format syntax is supported by both methods.
 *
 * Simple operations are supported to compare dates, or to calculate
 * with dates.
 *
 * <h3>i18n</h3>
 *
 * If the thread using a WDate is bound to a WApplication (i.e.
 * WApplication::instance() does not return 0), the date strings
 * can be localized by overriding the default values for
 * the localized string keys in the resource bundles of the WApplication:
 * - Short day names: Wt.WDate.Mon through Wt.WDate.Sun
 * - Long day names: Wt.WDate.Monday through Wt.WDate.Sunday
 * - Short month names: Wt.WDate.Jan through Wt.WDate.Dec
 * - Long month names: Wt.WDate.January through Wt.WDate.December
 *
 * If the thread using a WDate is not bound to a WApplication (i.e.
 * WApplication::instance() returns 0), english strings will be used.
 *
 * Internationalization affects both WDate to string conversions and
 * string to WDate conversion.
 */
class WT_API WDate
{
public:
  /*! \brief Construct a <i>Null</i> date.
   *
   * A date for which isNull() returns \c true. A \p Null date is also
   * invalid.
   *
   * \sa isValid(), isNull()
   */
  WDate();

  /*! \brief Construct a date with a time_point.
   */
  explicit WDate(const std::chrono::system_clock::time_point& tp);

  /*! \brief Specify a date by year, month, and day.
   *
   * The \p month has range 1-12 and the \p day has range 1-31.
   * When the date is invalid, isValid() is set to \c false.
   *
   * \sa setDate(), year(), month(), day()
   */
  WDate(int year, int month, int day);

  /*! \brief Sets the date by year, month, and day.
   *
   * The \p month has range 1-12 and the \p day has range 1-31.
   * When the new date is invalid, isValid() will return \c false.
   *
   * \sa WDate(int year, int month, int day), year(), month(), day()
   */
  void setDate(int year, int month, int day);

  /*! \brief Sets the date from a time point.
   */
  void setTimePoint(const std::chrono::system_clock::time_point& tp);

  /*! \brief Adds days.
   *
   * Returns a date that is \p ndays later than this
   * date. Negative values for \p ndays will result in a date that
   * is as many days earlier.
   *
   * Returns a null date if the current date is invalid or the new
   * date is out of range.
   *
   * \sa addMonths(), addYears()
   */
  WDate addDays(int ndays) const;

  /*! \brief Adds months.
   *
   * Returns a date that is the same day of the month, but
   * \p nmonths later than this date. Negative values for
   * \p nmonths will result in a date that is as many months
   * earlier.
   *
   * If the day does not exist in the resulting month/year
   * then the date is set to the last day of that month
   * (e.g. 2019-05-31 + 1 month = 2019-06-30)
   *
   * Returns a null date if the current date is invalid or the new
   * date is out of range.
   *
   * \sa addDays(), addYears()
   */
  WDate addMonths(int nmonths) const;

  /*! \brief Adds years.
   *
   * Returns a date that is \p nyears later than this
   * date. Negative values for \p nyears will result in a date
   * that is as many years earlier.
   *
   * If the day does not exist in the resulting month/year
   * then the date is set to the last day of that month
   * (e.g. 2016-02-29 + 1 year = 2017-02-28)
   *
   * Returns a null date if the current date is invalid or the new
   * date is out of range.
   *
   * \sa addDays(), addMonths()
   */
  WDate addYears(int nyears) const;

  /*! \brief Returns if this date is <i>Null</i>.
   *
   * A null date is also invalid.
   *
   * \sa isValid(), WDate()
   */
  bool isNull() const { return ymd_ == 0; }

  /*! \brief Returns if this date is valid.
   *
   * \sa isNull(), WDate(int, int, int), setDate()
   */
  bool isValid() const { return ymd_ > 1; }

  /*! \brief Returns the year.
   *
   * Returns 0 if the date is invalid.
   */
  int year() const { return (ymd_ >> 16); }

  /*! \brief Returns the month (1-12).
   *
   * Returns 0 if the date is invalid.
   */
  int month() const { return (ymd_ >> 8) & 0xFF; }

  /*! \brief Returns the day of month (1-31).
   *
   * Returns 0 if the date is invalid.
   */
  int day() const { return ymd_ & 0xFF; }

  /*! \brief Returns the day of week (1-7).
   *
   * Returns the day of week, from Monday (=1) to Sunday (=7).
   *
   * Returns 0 if the date is invalid.
   */
  int dayOfWeek() const;

  /*! \brief Returns the difference between two dates (in days).
   *
   * Returns 0 if either date is invalid.
   */
  int daysTo(const WDate& date) const;

  /*! \brief Converts the date to a Julian day. 
   *
   * Returns 0 if the date is invalid.
   *
   * \sa fromJulianDay()
   */
  int toJulianDay() const;

  /*! \brief Compares two dates.
   */
  bool operator< (const WDate& other) const;

  /*! \brief Compares two dates.
   */
  bool operator<= (const WDate& other) const;

  /*! \brief Compares two dates.
   */
  bool operator> (const WDate& other) const;

  /*! \brief Compares two dates.
   */
  bool operator>= (const WDate& other) const;

  /*! \brief Compares two dates.
   */
  bool operator== (const WDate& other) const;

  /*! \brief Compares two dates.
   */
  bool operator!= (const WDate& other) const;

  static WT_USTRING defaultFormat();

  /*! \brief Formats this date to a string using a default format.
   *
   * The default \p format is "ddd MMM d yyyy".
   * For example, a date constructed as:
   * \code
   *   WDate d(2007,8,29);
   * \endcode
   * will be formatted as:
   * \code
   *   "Wed Aug 29 2007"
   * \endcode
   *
   * \sa toString(const WString& format) const, fromString()
   */
  WT_USTRING toString() const;

  /*! \brief Formats this date to a string using a specified format.
   *
   * The \p format is a string in which the following contents has
   * a special meaning.
   *
   * <table>
   *  <tr><td><b>Code</b></td><td><b>Meaning</b></td>
   *      <td><b>Example (for Mon Aug 3 2007)</b></td></tr>
   *  <tr><td>d</td><td>The day without leading zero (1-31)</td>
          <td>3</td></tr>
   *  <tr><td>dd</td><td>The day with leading zero (01-31)</td>
          <td>03</td></tr>
   *  <tr><td>ddd</td><td>The day abbreviated using shortDayName()</td>
          <td>Mon</td></tr>
   *  <tr><td>dddd</td><td>The day abbreviated using longDayName()</td>
          <td>Monday</td></tr>
   *  <tr><td>M</td><td>The month without leading zero (1-12)</td>
          <td>8</td></tr>
   *  <tr><td>MM</td><td>The month with leading zero (01-12)</td>
          <td>08</td></tr>
   *  <tr><td>MMM</td><td>The month abbreviated using shortMonthName()</td>
          <td>Aug</td></tr>
   *  <tr><td>MMMM</td><td>The month abbreviated using longMonthName()</td>
          <td>August</td></tr>
   *  <tr><td>yy</td><td>The year as a two-digit number (00-99)</td>
          <td>07</td></tr>
   *  <tr><td>yyyy</td><td>The year as a four-digit number (-9999-9999)</td>
          <td>2007</td></tr>
   * </table>
   *
   * Any other text is kept literally. String content between single
   * quotes (') are not interpreted as special codes. LabelOption::Inside a string, a literal
   * quote may be specifed using a double quote ('').
   *
   * Examples of format and result:
   * <table>
   *  <tr><td><b>Format</b></td><td><b>Result (for Mon Aug 3 2007)</b></td></tr>
   *  <tr><td>ddd MMM d yyyy</td><td>Mon Aug 3 2007</td></tr>
   *  <tr><td>dd/MM/yyyy</td><td>03/08/2007</td></tr>
   *  <tr><td>dddd, MMM d, yyyy</td><td>Wednesday, Aug 3, 2007</td></tr>
   *  <tr><td>'MM': MM, 'd': d, 'yyyy': yyyy</td><td>MM: 08, d: 3, yyyy: 2007</td></tr>
   * </table>
   *
   * \sa fromString(const WString& value, const WString& format)
   */
  WT_USTRING toString(const WT_USTRING& format) const;

  std::chrono::system_clock::time_point toTimePoint() const;

  /*! \brief Parses a string to a date using a default format.
   *
   * The default \p format is "ddd MMM d yyyy".
   * For example, a date specified as:
   * \code
   *   "Wed Aug 29 2007"
   * \endcode
   * will be parsed as a date that equals a date constructed as:
   * \code
   *   WDate d(2007,8,29);
   * \endcode
   *
   * When the date could not be parsed or is not valid, an invalid
   * date is returned (for which isValid() returns false).
   *
   * \sa fromString(const WString& s, const WString& format), isValid()
   */
  static WDate fromString(const WT_USTRING& s);

  /*! \brief Parses a string to a date using a specified format.
   *
   * The \p format follows the same syntax as used by
   * \link toString(const WString& format) const toString(const WString& format)\endlink.
   *
   * When the date could not be parsed or is not valid, an invalid
   * date is returned (for which isValid() returns false). 
   *
   * \sa toString(const WString&) const
   */
  static WDate fromString(const WT_USTRING& s, const WT_USTRING& format);

  /*! \brief Converts a Julian Day <i>jd</i> to a \link Wt::WDate WDate \endlink.
   *
   * \sa toJulianDay() const
   */
  static WDate fromJulianDay(int jd);

  /*! \brief Reports the current client date.
   *
   * This method uses browser information to retrieve the date that is
   * configured in the client.
   *
   * \sa WLocalDateTime::currentDate()
   */
  static WDate currentDate();

  /*! \brief Reports the current server date.
   *
   * This method returns the local date on the server.
   *
   * \sa WDateTime::currentDateTime(), WLocalDateTime::currentServerDateTime()
   */
  static WDate currentServerDate();

  /*! \brief Returns whether the given year is a leap year.
   *
   * A leap year has 29 days in February, in case you wondered.
   */
  static bool isLeapYear(int year);

  /*! \brief Returns a date object representing the previous weekday
   *
   * Weekday [1 - 7]
   */
  static WDate previousWeekday(WDate& d, int weekday);

  static bool isValid(int year, int month, int day);

  /*! \brief Returns the short day name.
   *
   * Results (for given \p weekDay) are (default English):<br>
   * "Mon" (1),<br> "Tue" (2),<br> "Wed" (3),<br>
   * "Thu" (4),<br> "Fri" (5),<br> "Sat" (6),<br> "Sun" (7).
   *
   * The result is affected by localization using the "Wt.WDate.Mon" to
   * "Wt.WDate.Sun" keys.
   *
   * \sa longDayName()
   */
  static WT_USTRING shortDayName(int weekday, bool localized = true);

  /*! \brief Returns the short month name.
   *
   * Results (for given \p month) are (default English):<br>
   * "Jan" (1),<br> "Feb" (2),<br> "Mar" (3),<br>
   * "Apr" (4),<br> "May" (5),<br> "Jun" (6),<br>
   * "Jul" (7),<br> "Aug" (8),<br> "Sep" (9),<br>
   * "Oct" (10),<br> "Nov" (11),<br> "Dec" (12)<br>.
   *
   * The result is affected by localization using the "Wt.WDate.Jan" to
   * "Wt.WDate.Dec" keys.
   *
   * \sa longMonthName()
   */
  static WT_USTRING shortMonthName(int month, bool localized = true);

  /*! \brief Returns the long day name.
   *
   * Results (for given \p weekDay) are (default English):<br>
   * "Monday" (1),<br> "Tuesday" (2),<br> "Wednesday" (3),<br>
   * "Thursday" (4),<br> "Friday" (5),<br> "Saturday" (6),<br> "Sunday" (7).
   *
   * The result is affected by localization using the "Wt.WDate.Monday" to
   * "Wt.WDate.Sunday" keys.
   *
   * \sa shortDayName()
   */
  static WT_USTRING longDayName(int weekday, bool localized = true);

  /*! \brief Returns the long month name.
   *
   * Results (for given \p month) are (default English):<br>
   * "January" (1),<br> "February" (2),<br> "March" (3),<br>
   * "April" (4),<br> "May" (5),<br> "June" (6),<br> "July" (7),<br>
   * "August" (8),<br> "September" (9),<br> "October" (10),<br>
   * "November" (11),<br> "December" (12).
   *
   * The result is affected by localization using the "Wt.WDate.January" to
   * "Wt.WDate.December" keys.
   *
   * \sa shortDayName()
   */
  static WT_USTRING longMonthName(int month, bool localized = true);

  static std::string extFormat(const WT_USTRING& format);

  struct RegExpInfo {
    std::string regexp;
    std::string dayGetJS;
    std::string monthGetJS;
    std::string yearGetJS;
  };

  static RegExpInfo formatToRegExp(const WT_USTRING& format);

private:
  unsigned ymd_;

  void setYmd(int year, int month, int day);

  struct ParseState {
    int d, M, y;
    int day, month, year;

    ParseState();
  };

  static bool parseLast(const std::string& v, unsigned& vi,
			ParseState& state, const WString& format);

  static WDateTime::CharState handleSpecial(char c, const std::string& v,
					    unsigned& vi, ParseState& parse,
					    const WString& format);

  bool writeSpecial(const std::string& f, unsigned& i, WStringStream& result,
                    bool localized = true)
    const;

  static int parseShortMonthName(const std::string& v, unsigned& pos);
  static int parseLongMonthName(const std::string& v, unsigned& pos);
  static int parseShortDayName(const std::string& v, unsigned& pos);
  static int parseLongDayName(const std::string& v, unsigned& pos);

  friend class WDateTime;
};

}

#endif // WDATE_H_
