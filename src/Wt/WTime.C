/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <math.h>
#include <cstdio>

#include "Wt/WException.h"
#include "Wt/WLocalDateTime.h"
#include "Wt/WTime.h"
#include "Wt/WLogger.h"
#include "Wt/Date/date.h"

#include "WebUtils.h"

namespace Wt {

LOGGER("WTime");

WTime::WTime()
  : valid_(false),
    null_(true),
    time_(0)
{ }

WTime::WTime(int h, int m, int s, int ms)
  : valid_(false),
    null_(false),
    time_(0)
{
  setHMS(h, m, s, ms);
}

bool WTime::setHMS(int h, int m, int s, int ms)
{
  null_ = false;
  if (   m >= 0 && m <= 59
         && s >= 0 && s <= 59
         && ms >= 0 && ms <= 999) {
    valid_ = true;
    bool negative = h < 0;

    if (negative)
      h = -h;

    time_ = ((h * 60 + m) * 60 + s) * 1000 + ms;

    if (negative)
      time_ = -time_;
  } else {
    LOG_WARN("Invalid time: " << h << ":" << m << ":" << s << "." << ms);
  }

  return valid_;
}

WTime WTime::addMSecs(int ms) const
{
  if (valid_)
    return WTime(time_ + ms);
  else
    return *this;
}

WTime WTime::addSecs(int s) const
{
  return addMSecs(1000 * s);
}

int WTime::hour() const
{
  return time_ / (1000 * 60 * 60);
}

int WTime::minute() const
{
  return (std::abs((int)time_) / (1000 * 60)) % 60;
}

int WTime::second() const
{
  return (std::abs((int)time_) / 1000) % 60;
}

int WTime::msec() const
{
  return std::abs((int)time_) % 1000;
}

long WTime::secsTo(const WTime& t) const
{
  return msecsTo(t) / 1000;
}

long WTime::msecsTo(const WTime& t) const
{
  if (isValid() && t.isValid())
    return t.time_ - time_;
  else
    return 0;
}

bool WTime::operator> (const WTime& other) const
{
  return other < *this;
}

bool WTime::operator< (const WTime& other) const
{
  if (isValid() && other.isValid())
    return time_ < other.time_;
  else
    return false;
}

bool WTime::operator!= (const WTime& other) const
{
  return !(*this == other);
}

bool WTime::operator== (const WTime& other) const
{
  return valid_ == other.valid_ && null_ == other.null_ && time_ == other.time_;
}

bool WTime::operator<= (const WTime& other) const
{
  return (*this == other) || (*this < other);
}

bool WTime::operator>= (const WTime& other) const
{
  return other <= *this;
}

WTime WTime::currentTime()
{
  return WLocalDateTime::currentDateTime().time();
}

WTime WTime::currentServerTime()
{
  return WLocalDateTime::currentServerDateTime().time();
}

WString WTime::defaultFormat()
{
  return WString::fromUTF8("HH:mm:ss");
}

WTime WTime::fromString(const WString& s)
{
  return fromString(s, defaultFormat());
}

WTime::ParseState::ParseState()
{
  h = m = s = z = a = 0;
  hour = minute = sec = msec = 0;
  pm = parseAMPM = haveAMPM = false;
}

WTime WTime::fromString(const WString& s, const WString& format)
{
  WTime result;
  WDateTime::fromString(nullptr, &result, s, format);
  return result;
}

WDateTime::CharState WTime::handleSpecial(char c, const std::string& v,
    unsigned& vi, ParseState& parse,
    const WString& format)
{
  switch (c) {
    case 'H':
    case 'h':
      parse.parseAMPM = c == 'h';

      if (parse.h == 0)
        if (!parseLast(v, vi, parse, format))
          return WDateTime::CharState::CharInvalid;

      ++parse.h;
      return WDateTime::CharState::CharHandled;

    case 'm':
      if (parse.m == 0)
        if (!parseLast(v, vi, parse, format))
          return WDateTime::CharState::CharInvalid;

      ++parse.m;
      return WDateTime::CharState::CharHandled;

    case 's':
      if (parse.s == 0)
        if (!parseLast(v, vi, parse, format))
          return WDateTime::CharState::CharInvalid;

      ++parse.s;
      return WDateTime::CharState::CharHandled;

    case 'z':
      if (parse.z == 0)
        if (!parseLast(v, vi, parse, format))
          return WDateTime::CharState::CharInvalid;

      ++parse.z;
      return WDateTime::CharState::CharHandled;

    case 'A':
    case 'a':
      if (!parseLast(v, vi, parse, format))
        return WDateTime::CharState::CharInvalid;

      parse.a = 1;
      return WDateTime::CharState::CharHandled;

    case 'P':
    case 'p':
      if (parse.a == 1) {
        if (!parseLast(v, vi, parse, format))
          return WDateTime::CharState::CharInvalid;

        return WDateTime::CharState::CharHandled;
      }

    /* fall through */

    default:
      if (!parseLast(v, vi, parse, format))
        return WDateTime::CharState::CharInvalid;

      return WDateTime::CharState::CharUnhandled;
  }
}

static void fatalFormatError(const WString& format, int c, const char *cs)
{
  std::stringstream s;
  s << "WTime format syntax error (for \"" << format.toUTF8()
    << "\"): Cannot handle " << c << " consecutive " << cs;
  throw WException(s.str());
}

bool WTime::parseLast(const std::string& v, unsigned& vi,
                      ParseState& parse,
                      const WString& format)
{
  const char *letter[] = { "h's", "m's", "s'es", "z's" };

  for (int i = 0; i < 4; ++i) {
    int *count;
    int *value;
    int maxCount = 2;

    switch (i) {
      case 0:
        count = &parse.h;
        value = &parse.hour;
        break;

      case 1:
        count = &parse.m;
        value = &parse.minute;
        break;

      case 2:
        count = &parse.s;
        value = &parse.sec;
        break;

      case 3:
        count = &parse.z;
        value = &parse.msec;
        maxCount = 3;
    }

    if (*count != 0) {
      if (*count == 1) {
        std::string str;

        if (vi >= v.length())
          return false;

        if ((i == 0) && (v[vi] == '-' || v[vi] == '+')) {
          str += v[vi++];

          if (vi >= v.length())
            return false;
        }

        str += v[vi++];

        for (int j = 0; j < maxCount - 1; ++j)
          if (vi < v.length())
            if ('0' <= v[vi] && v[vi] <= '9')
              str += v[vi++];

        try {
          *value = Utils::stoi(str);
        } catch (std::exception&) {
          return false;
        }
      } else if (*count == maxCount) {
        if (vi + (maxCount - 1) >= v.length())
          return false;

        std::string str = v.substr(vi, maxCount);
        vi += maxCount;

        try {
          *value = Utils::stoi(str);
        } catch (std::exception&) {
          return false;
        }
      } else
        fatalFormatError(format, *count, letter[i]);
    }

    *count = 0;
  }

  if (parse.a) {
    if (vi + 1 >= v.length())
      return false;

    std::string str = v.substr(vi, 2);
    vi += 2;
    parse.haveAMPM = true;

    if (str == "am" || str == "AM")
      parse.pm = false;
    else if (str == "pm" || str == "PM")
      parse.pm = true;
    else
      return false;

    parse.a = 0;
  }

  return true;
}

WString WTime::toString() const
{
  return WTime::toString(defaultFormat());
}

WString WTime::toString(const WString& format) const
{
  return WDateTime::toString(nullptr, this, format, true, 0);
}

std::chrono::duration<int, std::milli> WTime::toTimeDuration() const
{
  if(isValid())
      return std::chrono::duration<int, std::milli>(std::chrono::hours(hour()) + std::chrono::minutes(minute()) + std::chrono::seconds(second())
              + std::chrono::milliseconds(msec()));
  return std::chrono::duration<int, std::milli>(0);
}

WTime WTime::fromTimeDuration(const std::chrono::duration<int, std::milli>& duration)
{
    auto time = date::make_time(duration);
    std::chrono::duration<int, std::milli> ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.subseconds());
    return WTime(time.hours().count(), time.minutes().count(), time.seconds().count(), ms.count());
}

int WTime::pmhour() const
{
  int result = hour() % 12;
  return result != 0 ? result : 12;
}

bool WTime::writeSpecial(const std::string& f, unsigned& i,
                         WStringStream& result, bool useAMPM,
                         int zoneOffset) const
{
  char buf[30];

  switch (f[i]) {
    case '+':
      if (f[i + 1] == 'h' || f[i + 1] == 'H') {
        result << ((hour() >= 0) ? '+' : '-');
        return true;
      }

      return false;

    case 'h':
      if (f[i + 1] == 'h') {
        ++i;
        result << Utils::pad_itoa(std::abs(useAMPM ? pmhour() : hour()), 2, buf);
      } else
        result << Utils::itoa(std::abs(useAMPM ? pmhour() : hour()), buf);

      return true;

    case 'H':
      if (f[i + 1] == 'H') {
        ++i;
        result << Utils::pad_itoa(std::abs(hour()), 2, buf);
      } else
        result << Utils::itoa(std::abs(hour()), buf);

      return true;

    case 'm':
      if (f[i + 1] == 'm') {
        ++i;
        result << Utils::pad_itoa(minute(), 2, buf);
      } else
        result << Utils::itoa(minute(), buf);

      return true;

    case 's':
      if (f[i + 1] == 's') {
        ++i;
        result << Utils::pad_itoa(second(), 2, buf);
      } else
        result << Utils::itoa(second(), buf);

      return true;

    case 'Z': {
        bool negate = zoneOffset < 0;

        if (!negate)
          result << '+';
        else {
          result << '-';
          zoneOffset = -zoneOffset;
        }

        int hours = zoneOffset / 60;
        int minutes = zoneOffset % 60;
        result << Utils::pad_itoa(hours, 2, buf);
        result << Utils::pad_itoa(minutes, 2, buf);
        return true;
      }

    case 'z':
      if (f.substr(i + 1, 2) == "zz") {
        i += 2;
        result << Utils::pad_itoa(msec(), 3, buf);
      } else
        result << Utils::itoa(msec(), buf);

      return true;

    case 'a':
    case 'A':
      if (hour() < 12)
        result << ((f[i] == 'a') ? "am" : "AM");
      else
        result << ((f[i] == 'a') ? "pm" : "PM");

      if (f[i + 1] == 'p' || f[i + 1] == 'P')
        ++i;

      return true;

    default:
      return false;
  }
}

WTime::WTime(long t)
  : valid_(true),
    null_(false),
    time_(t)
{ }

WTime::RegExpInfo WTime::formatHourToRegExp(WTime::RegExpInfo& result,
					    const std::string& format,
                        unsigned& i, int& currentGroup)
{
  /* Possible values */
  /* h, hh, H, HH */
  char next = -1;

  bool ap = (format.find("AP") != std::string::npos)
    || (format.find("ap") != std::string::npos); //AM-PM
  std::string sf;
  sf += format[i];

  if (i < format.size() - 1)
    next = format[i + 1];

  if (next == 'h' || next == 'H') {
    sf+= next;
    i++;
  } else{
    next = -1;
    sf = format[i];
  }
  
  if (sf == "HH" || (sf == "hh" && !ap)) { //Hour with leading 0 0-23
    result.regexp += "([0-1][0-9]|[2][0-3])";
  } else if (sf == "hh" && ap) {          //Hour with leading 0 01-12
    result.regexp += "(0[1-9]|[1][012])";
  } else if (sf == "H" || (sf == "h" && !ap)) { //Hour without leading 0 0-23
	result.regexp += "(0|[1-9]|[1][0-9]|2[0-3])";
  } else if (sf == "h" && ap) {                  //Hour without leading 0 0-12
    result.regexp += "([1-9]|1[012])";
  }
  result.hourGetJS = "return parseInt(results[" + 
    std::to_string(currentGroup++) + "], 10);";
  return result;
}

WTime::RegExpInfo WTime::formatMinuteToRegExp(WTime::RegExpInfo& result,
					      const std::string& format,
                          unsigned& i, int& currentGroup)
{
  char next = -1;
  std::string sf;
  if (i < format.size() - 1) next = format[i + 1];
  
  if (next == 'm') {
    sf = "mm";
    i++;
  } else {
    sf = "m";
    next = -1;
  }
  
  if (sf == "m") /* Minutes without leading 0 */
    result.regexp += "(0|[1-5]?[0-9])";
  else /* Minutes with leading 0 */
    result.regexp += "([0-5][0-9])";

  result.minuteGetJS = "return parseInt(results["
          + std::to_string(currentGroup++) + "], 10);";

  return result;
}

WTime::RegExpInfo WTime::formatSecondToRegExp(WTime::RegExpInfo& result,
					      const std::string& format,
                          unsigned& i, int& currentGroup)
{
  char next = -1;
  std::string sf;

  if (i < format.size() - 1) next = format[i + 1];
  
  if (next == 's') {
    sf = "ss";
    i++;
  } else {
    sf = "s";
    next = -1;
  }
  
  if (sf == "s") /* Seconds without leading 0 */
    result.regexp += "(0|[1-5]?[0-9])";
  else /* Seconds with leading 0 */
    result.regexp += "([0-5][0-9])";

  result.secGetJS = "return parseInt(results["
          + std::to_string(currentGroup++) + "], 10);";

  return result;
}

WTime::RegExpInfo WTime::formatMSecondToRegExp(WTime::RegExpInfo& result,
					       const std::string& format,
                           unsigned& i, int& currentGroup)
{
  char next = -1;
  std::string sf;
  sf += format[i];

  for (int k = 0; k < 2; ++k) {
    if (i < format.size() - 1) next = format[i + 1];

    if (next == 'z'){
      sf += "z";
      next = -1;
      i++;
    } else {
      next = -1;
      break;
    }
  }

  if (sf == "z") /* The Ms without trailing 0 */
    result.regexp += "(0|[1-9][0-9]{0,2})";
  else if (sf == "zzz") 
    result.regexp += "([0-9]{3})";

  result.msecGetJS = "return parseInt(results["
          + std::to_string(currentGroup++) + "], 10);";

  return result;
}

WTime::RegExpInfo WTime::formatAPToRegExp(WTime::RegExpInfo& result,
					       const std::string& format,
					       unsigned& i)
{
  if(i < format.size() - 1) {
	if(format[i] ==  'A' && format[i+1] == 'P') {
	  result.regexp += "([AP]M)";
	  i++;
	}
	else if(format[i]  == 'a' && format[i+1] == 'p'){
	  result.regexp += "([ap]m)";
	  i++;
	}
  } else 
	result.regexp += format[i];

  return result;
}

WTime::RegExpInfo WTime::processChar(WTime::RegExpInfo &result, const std::string& format, unsigned& i)
{
  switch(format[i])
  {
	case '.':
	case '+':
	case '$':
	case '^':
	case '*':
	case '[':
	case ']':
	case '{':
	case '}':
	case '(':
	case ')':
	case '?':
	case '!':
	  result.regexp += "\\";
	  break;
  }
  result.regexp += format[i];
  return result;
}

WTime::RegExpInfo WTime::formatToRegExp(const WT_USTRING& format)
{
  RegExpInfo result;
  std::string f = format.toUTF8();
  int currentGroup = 1;

  result.hourGetJS = "return 1";
  result.minuteGetJS = "return 1";
  result.secGetJS = "return 1";
  result.msecGetJS = "return 1";

  //result.regexp = "^";
  bool inQuote = false;

  for (unsigned i = 0; i < f.size(); ++i) {
    if (inQuote && f[i] != '\'') {
	  processChar(result, f, i);
      continue;
    }

    switch (f[i]) {
    case '\'':
      if (i < f.size() - 2 && f[i+1] == f[i+2] && f[i+1] == '\'')
        result.regexp += f[i];
      else 
        inQuote = !inQuote;
    case 'h':
    case 'H':
      formatHourToRegExp(result, f, i, currentGroup);
      break;
    case 'm':
      formatMinuteToRegExp(result,f, i, currentGroup);
      break;
    case 's':
      formatSecondToRegExp(result, f, i, currentGroup);
      break;
    case 'z':
      formatMSecondToRegExp(result, f, i, currentGroup);
      break;
    case 'Z':
      result.regexp+="(\\+[0-9]{4})";
      break;
    case 'A':
    case 'a':
	  formatAPToRegExp(result, f, i);
	  break;
    case '+':
      if (i < f.size() - 1 && (f[i+1] == 'h' || f[i+1] == 'H'))
        result.regexp += "\\+";
      break;
    default:
	  processChar(result, f, i);
	  break;
    }

  }

  //result.regexp += "$";

  return result;
}

bool WTime::usesAmPm(const WString& format)
{
  std::string f = format.toUTF8() + std::string(3, 0);

  bool inQuote = false;
  bool gotQuoteInQuote = false;
  bool useAMPM = false;

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
  
  return useAMPM;
}

}
