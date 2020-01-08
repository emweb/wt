// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTIME_H_
#define WTIME_H_

#include <Wt/WDateTime.h>
#include <Wt/WString.h>
#include <Wt/WStringStream.h>
#include <exception>

namespace Wt {

/*! \class WTime Wt/WTime.h Wt/WTime.h
 *  \brief A value class that defines a clock time.
 *
 * A clock time represents the time of day (usually 0 to 24 hour), up
 * to millisecond precision.
 *
 * As of version %3.3.1, the time class itself will no longer limit times
 * to the 24-hour range, but will allow any time (duration), including negative
 * values.
 *
 * \sa WDate, WDateTime
 */
class WT_API WTime
{
public:
  /*! \brief Construct a <i>Null</i> time.
   *
   * A time for which isNull() returns true. A <i>Null</i> time is also
   * invalid.
   *
   * \sa isValid(), isNull()
   */
  WTime();

  /*! \brief Construct a time given hour, minutes, seconds, and milliseconds.
   *
   * \p m and \p s have range 0-59, and \p ms has range 0-999. A duration can
   * be positive or negative depending on the sign of \p h.
   *
   * When the time is invalid, isValid() is set to \c false.
   */
  WTime(int h, int m, int s = 0, int ms = 0);

  /*! \brief Sets the time.
   *
   * \p m and \p s have range 0-59, and \p ms has range 0-999.
   *
   * When the time is invalid, isValid() is set to \c false.
   */
  bool setHMS(int h, int m, int s, int ms = 0);

  /*! \brief Adds seconds.
   *
   * Returns a time that is \p s seconds later than this time. Negative
   * values for \p s will result in a time that is as many seconds
   * earlier.
   */
  WTime addSecs(int s) const;

  /*! \brief Adds milliseconds.
   *
   * Returns a time that is \p ms milliseconds later than this
   * time. Negative values for \p ms will result in a time that
   * is as many milliseconds earlier.
   */
  WTime addMSecs(int ms) const;

  /*! \brief Returns if this time is <i>Null</i>.
   *
   * A null time is also invalid.
   *
   * \sa isValid(), WTime()
   */
  bool isNull() const { return null_; }

  /*! \brief Returns if this time is valid.
   */
  bool isValid() const { return valid_; }

  /*! \brief Returns the hour.
   */
  int hour() const;

  /*! \brief Returns the minutes (0-59).
   */
  int minute() const;

  /*! \brief Returns the seconds (0-59).
   */
  int second() const;

  /*! \brief Returns the milliseconds (0-999)
   */
  int msec() const;

  /*! \brief Returns the difference between two time values (in seconds).
   *
   * The result is negative if t is earlier than this.
   */
  long secsTo(const WTime& t) const;

  /*! \brief Returns the difference between two time values (in milliseconds).
   *
   * The result is negative if t is earlier than this.
   */
  long msecsTo(const WTime& t) const;

  /*! \brief Compares two time values.
   */
  bool operator< (const WTime& other) const;

  /*! \brief Compares two time values.
   */
  bool operator<= (const WTime& other) const;

  /*! \brief Compares two time values.
   */
  bool operator> (const WTime& other) const;

  /*! \brief Compares two time values.
   */
  bool operator>= (const WTime& other) const;

  /*! \brief Compares two time values.
   */
  bool operator== (const WTime& other) const;

  /*! \brief Compares two time values.
   */
  bool operator!= (const WTime& other) const;

  static WT_USTRING defaultFormat();

  /*! \brief Formats this time to a string using a default format.
   *
   * The default format is "hh:mm:ss".
   */
  WT_USTRING toString() const;

  /*! \brief Formats this time to a string using a specified format.
   *
   * The \p format is a string in which the following contents has
   * a special meaning.
   *
   * <table>
   *  <tr><td><b>Code</b></td><td><b>Meaning</b></td>
   *      <td><b>Example (for 14:06:23.045)</b></td></tr>
   *  <tr><td>h</td><td>The hour without leading zero (0-23 or 1-12 for AM/PM display)</td>
          <td>14 or 2</td></tr>
   *  <tr><td>hh</td><td>The hour with leading zero (00-23 or 01-12 for AM/PM display)</td>
          <td>14 or 02</td></tr>
   *  <tr><td>H</td><td>The hour without leading zero (0-23)</td>
          <td>14</td></tr>
   *  <tr><td>HH</td><td>The hour with leading zero (00-23)</td>
          <td>14</td></tr>
   *  <tr><td>+ followed by (h/hh/H/HH)</td><td>The sign of the hour (+/-)</td>
          <td>+</td></tr>
   *  <tr><td>m</td><td>The minutes without leading zero (0-59)</td>
          <td>6</td></tr>
   *  <tr><td>mm</td><td>The minutes with leading zero (00-59)</td>
          <td>06</td></tr>
   *  <tr><td>s</td><td>The seconds without leading zero (0-59)</td>
          <td>23</td></tr>
   *  <tr><td>ss</td><td>The seconds with leading zero (00-59)</td>
          <td>23</td></tr>
   *  <tr><td>z</td><td>The milliseconds without leading zero (0-999)</td>
          <td>45</td></tr>
   *  <tr><td>zzz</td><td>The millisecons with leading zero (000-999)</td>
          <td>045</td></tr>
   *  <tr><td>AP or A</td><td>use AM/PM display: affects h or hh display and is replaced itself by AM/PM</td>
          <td>PM</td></tr>
   *  <tr><td>ap or a</td><td>use am/pm display: affects h or hh display and is replaced itself by am/pm</td>
          <td>pm</td></tr>
   *  <tr><td>Z</td><td>the timezone in RFC 822 format (e.g. -0800)</td>
          <td>+0000</td></tr>
   * </table>
   *
   * Any other text is kept literally. String content between single
   * quotes (') are not interpreted as special codes. LabelOption::Inside a string, a literal
   * quote may be specifed using a double quote ('').
   *
   * Examples of format and result:
   * <table>
   *  <tr><td><b>Format</b></td><td><b>Result (for 22:53:13.078)</b></td></tr>
   *  <tr><td>hh:mm:ss.zzz</td><td>22:53:13.078</td></tr>
   *  <tr><td>hh:mm:ss AP</td><td>10:53:13 PM</td></tr>
   * </table>
   *
   * \sa fromString(const WString& value, const WString& format)
   */
  WT_USTRING toString(const WT_USTRING& format) const;

  /*! \brief Parses a string to a time using a default format.
   *
   * The default format is "hh:mm:ss".
   * For example, a time specified as:
   * \code
   *   "22:55:15"
   * \endcode
   * will be parsed as a time that equals a time constructed as:
   * \code
   *   WTime d(22,55,15);
   * \endcode
   *
   * When the time could not be parsed or is not valid, an invalid
   * time is returned (for which isValid() returns false).
   *
   * \sa fromString(const WString& s, const WString& format), isValid()
   */
  static WTime fromString(const WT_USTRING& s);

  /*! \brief Parses a string to a time using a specified format.
   *
   * The \p format follows the same syntax as used by
   * \link toString(const WString& format) const toString(const WString& format)\endlink.
   *
   * When the time could not be parsed or is not valid, an invalid
   * time is returned (for which isValid() returns false). 
   *
   * \sa toString(const WString&) const
   */
  static WTime fromString(const WT_USTRING& s, const WT_USTRING& format);

  /*! \brief Reports the current client date.
   *
   * This method uses browser information to retrieve the time that is
   * configured in the client.
   *
   * \sa WLocalDateTime::currentDate()
   */
  static WTime currentTime();

  /*! \brief Reports the current server time.
   *
   * This method returns the local time on the server.
   *
   * \sa WDateTime::currentDateTime(), WLocalDateTime::currentServerDateTime()
   */
  static WTime currentServerTime();

  struct RegExpInfo {
    std::string regexp;
    std::string hourGetJS;
    std::string minuteGetJS;
    std::string secGetJS;
    std::string msecGetJS;
  };

  static RegExpInfo formatToRegExp(const WT_USTRING& format);

#ifndef WT_TARGET_JAVA
  std::chrono::duration<int, std::milli> toTimeDuration() const;
  static WTime fromTimeDuration(const std::chrono::duration<int, std::milli>& duration);
#else
  std::chrono::milliseconds toTimeDuration() const;
  static WTime fromTimeDuration(const std::chrono::milliseconds& duration);
#endif

private:
  bool valid_, null_;
  long time_;

  WTime (long time);

  struct ParseState {
    int h, m, s, z, a;
    int hour, minute, sec, msec;
    bool pm, parseAMPM, haveAMPM;

    ParseState();
  };

  static bool parseLast(const std::string& v, unsigned& vi,
			ParseState& parse, const WString& format);

  static WDateTime::CharState handleSpecial(char c, const std::string& v,
					    unsigned& vi, ParseState& parse,
					    const WString& format);

  bool writeSpecial(const std::string& f, unsigned& i, WStringStream& result,
		    bool useAMPM, int zoneOffset) const;

  int pmhour() const;

  static RegExpInfo formatHourToRegExp(RegExpInfo &result, const std::string& format, unsigned& i, int& currentGroup);
  static RegExpInfo formatMinuteToRegExp(RegExpInfo &result, const std::string& format, unsigned& i, int& currentGroup);
  static RegExpInfo formatSecondToRegExp(RegExpInfo &result, const std::string& format, unsigned& i, int& currentGroup);
  static RegExpInfo formatMSecondToRegExp(RegExpInfo &result, const std::string& format, unsigned& i, int& currentGroup);
  static RegExpInfo formatAPToRegExp(RegExpInfo &result, const std::string& format, unsigned& i);
  static RegExpInfo processChar(RegExpInfo &result, const std::string& format, unsigned& i);
  static bool usesAmPm(const WString& format);

  friend class WDateTime;
  friend class WTimePicker;
};

}

#endif // WTIME_H_
