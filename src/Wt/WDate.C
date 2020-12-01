/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WDate.h"
#include "Wt/WException.h"
#include "Wt/WLocalDateTime.h"
#include "Wt/WLogger.h"

#include "Wt/Date/date.h"

#include "WebUtils.h"
#include <cmath>

namespace {
  std::string WT_WDATE = "Wt.WDate.";
}

namespace Wt {

LOGGER("WDate");

WDate::WDate()
  : ymd_(0)
{ }

WDate::WDate(const std::chrono::system_clock::time_point& tp)
{
  setTimePoint(tp);
}

WDate::WDate(int year, int month, int day)
{
  setDate(year, month, day);
}

void WDate::setTimePoint(const std::chrono::system_clock::time_point& tp)
{
  date::sys_days dp = date::floor<date::days>(tp);
  auto ymd = date::year_month_day(dp);
  if(ymd.ok())
      setYmd((int)ymd.year(), (unsigned int)ymd.month(), (unsigned int)ymd.day());
  else
      ymd_ = 1; //not null and not valid
}

WDate WDate::addDays(int ndays) const
{
  if(isValid()){
      date::sys_days dp(date::day(day())/month()/year());
      date::year_month_day ymd(dp + date::days(ndays));
      return WDate((int)ymd.year(), (unsigned int)ymd.month(), (unsigned int)ymd.day());
  } else
      return WDate();
}

WDate WDate::addMonths(int nmonths) const
{
  if(isValid()){
      date::year_month_day ymd = date::day(day())/month()/year();
      ymd += date::months(nmonths);
      if(!ymd.ok() &&
         ymd.year().ok() &&
         ymd.month().ok() &&
         ymd.day() > (ymd.year() / ymd.month() / date::last).day())
        ymd = ymd.year() / ymd.month() / date::last;
      if(!ymd.ok())
          return WDate();
      return WDate((int)ymd.year(), (unsigned int)ymd.month(), (unsigned int)ymd.day());
  } else
      return WDate();
}

WDate WDate::addYears(int nyears) const
{
  if(isValid()){
      date::year_month_day ymd = date::day(day())/month()/year();
      ymd += date::years(nyears);
      if (!ymd.ok() &&
          ymd.year().ok() &&
          ymd.month().ok() &&
          ymd.day() > (ymd.year() / ymd.month() / date::last).day())
        ymd = ymd.year() / ymd.month() / date::last;
      if(!ymd.ok())
          return WDate();
      return WDate((int)ymd.year(), (unsigned int)ymd.month(), (unsigned int)ymd.day());
  } else
      return WDate();
}

void WDate::setDate(int year, int month, int day)
{
    date::year_month_day ymd = date::day(day)/month/year;
    if(!ymd.ok()){
        if(!ymd.year().ok())
            LOG_WARN("Invalid date: year not in range "
                     << (int)ymd.year().min()<< " .. "
                     << (int)ymd.year().max());
        if(!ymd.month().ok())
            LOG_WARN("Invalid date: month not in range 1 .. 12");
        if(!ymd.day().ok())
            LOG_WARN("Invalid date: day not in range 1 .. 31");
        ymd_ = 1;
        return;
    }
    setYmd(year, month, day);
}

void WDate::setYmd(int y, int m, int d)
{
  ymd_ = (y << 16) | ((m & 0xFF) << 8) | (d & 0xFF);
}

bool WDate::isLeapYear(int year)
{
    return date::year(year).is_leap();
}

int WDate::dayOfWeek() const
{
  if (!isValid())
    return 0;

  return date::weekday(date::day(day())/month()/year()).iso_encoding();
}

int WDate::daysTo(const WDate& other) const
{
  if (!isValid() || !other.isValid())
    return 0;

  date::year_month_day ymdthis = date::day(day())/month()/year();
  date::year_month_day ymdother = date::day(other.day())/other.month()/other.year();
  date::sys_days dpthis = ymdthis;
  date::sys_days dpother = ymdother;
  auto dd = dpother - dpthis;
  return dd.count();
}

int WDate::toJulianDay() const
{
  if (!isValid())
    return 0;
  else {
    int a = std::floor((14 - month())/12);
    int nyears = year() + 4800 - a;
    int nmonths = month() + 12*a -3;
    return day() + std::floor((153*nmonths + 2)/5) 
      + 365*nyears + std::floor(nyears/4) 
      - std::floor(nyears/100) + std::floor(nyears/400) - 32045;
  }
}

std::chrono::system_clock::time_point WDate::toTimePoint() const
{
  if (isValid()){
    date::year_month_day ymd = date::day(day())/month()/year();
    date::sys_days days = ymd;
    return days;
  }
  else
    return std::chrono::system_clock::time_point();
}

WDate WDate::previousWeekday(WDate &d, int weekday)
{
    if(!d.isValid())
        return WDate();
    WDate dt = d.addDays(-1);
    while(dt.dayOfWeek() != weekday)
        dt = dt.addDays(-1);
    return dt;
}

bool WDate::isValid(int year, int month, int day)
{
  WDate d(year, month, day);
  return d.isValid();
}

bool WDate::operator> (const WDate& other) const
{
  return other < *this;
}

bool WDate::operator< (const WDate& other) const
{
  return ymd_ < other.ymd_;
}

bool WDate::operator!= (const WDate& other) const
{
  return !(*this == other);
}

bool WDate::operator== (const WDate& other) const
{
  return ymd_ == other.ymd_;
}

bool WDate::operator<= (const WDate& other) const
{
  return (*this == other) || (*this < other);
}

bool WDate::operator>= (const WDate& other) const
{
  return other <= *this;
}

WDate WDate::currentServerDate()
{
  return WLocalDateTime::currentServerDateTime().date();
}

WDate WDate::currentDate()
{
  return WLocalDateTime::currentDateTime().date();
}

WString WDate::shortDayName(int weekday, bool localized)
{
  static const char *v[]
    = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

  if (localized && WApplication::instance())
    return WString::tr(WT_WDATE + "3." + v[weekday - 1]);
  else
    return WString::fromUTF8(v[weekday - 1]);
}

int WDate::parseShortDayName(const std::string& v, unsigned& pos)
{
  if (pos + 2 >= v.length())
    return -1;

  std::string d = v.substr(pos, 3);
  
  for (int i = 1; i <= 7; ++i) {
    if (d == shortDayName(i).toUTF8()) {
      pos += 3;
      return i;
    }
  }

  return -1;
}

WString WDate::longDayName(int weekday, bool localized)
{
  static const char *v[]
    = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
       "Sunday" };

  if (localized && WApplication::instance())
    return WString::tr(WT_WDATE + v[weekday - 1]);
  else
    return WString::fromUTF8(v[weekday - 1]);
}

int WDate::parseLongDayName(const std::string& v, unsigned& pos)
{
  std::string remainder = v.substr(pos);

  for (int i = 1; i <= 7; ++i) {
    std::string m = longDayName(i).toUTF8();

    if (remainder.length() >= m.length())
      if (remainder.substr(0, m.length()) == m) {
	pos += m.length();
	return i;
      }
  }

  return -1;
}

WString WDate::shortMonthName(int month, bool localized)
{
  static const char *v[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

  if (localized && WApplication::instance())
    return WString::tr(WT_WDATE + "3." + v[month - 1]);
  else
    return WString::fromUTF8(v[month - 1]);
}

int WDate::parseShortMonthName(const std::string& v, unsigned& pos)
{
  if (pos + 2 >= v.length())
    return -1;

  std::string m = v.substr(pos, 3);
  
  for (int i = 1; i <= 12; ++i) {
    if (m == shortMonthName(i).toUTF8()) {
      pos += 3;
      return i;
    }
  }

  return -1;
}

WString WDate::longMonthName(int month, bool localized)
{
  static const char *v[] = {"January", "February", "March", "April", "May",
			    "June", "July", "August", "September",
			    "October", "November", "December" };

  if (localized && WApplication::instance())
    return WString::tr(WT_WDATE + v[month - 1]);
  else
    return WString::fromUTF8(v[month - 1]);
}

int WDate::parseLongMonthName(const std::string& v, unsigned& pos)
{
  std::string remainder = v.substr(pos);

  for (int i = 1; i <= 12; ++i) {
    std::string m = longMonthName(i).toUTF8();

    if (remainder.length() >= m.length())
      if (remainder.substr(0, m.length()) == m) {
	pos += m.length();
	return i;
      }
  }

  return -1;
}

WString WDate::defaultFormat()
{
  return WString::fromUTF8("ddd MMM d yyyy"); 
}

WDate WDate::fromString(const WString& s)
{
  return fromString(s, defaultFormat());
}

WDate::ParseState::ParseState()
{
  d = M = y = 0;
  day = month = year = -1;
}

WDate WDate::fromString(const WString& s, const WString& format)
{
  WDate result;

  WDateTime::fromString(&result, nullptr, s, format);

  return result;
}

WDateTime::CharState WDate::handleSpecial(char c, const std::string& v,
					  unsigned& vi, ParseState& parse,
					  const WString& format)
{
  switch (c) {
  case 'd':
    if (parse.d == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharState::CharInvalid;

    ++parse.d;

    return WDateTime::CharState::CharHandled;

  case 'M':
    if (parse.M == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharState::CharInvalid;

    ++parse.M;

    return WDateTime::CharState::CharHandled;

  case 'y':
    if (parse.y == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharState::CharInvalid;

    ++parse.y;

    return WDateTime::CharState::CharHandled;

  default:
    if (!parseLast(v, vi, parse, format))
      return WDateTime::CharState::CharInvalid;

    return WDateTime::CharState::CharUnhandled;
  }
}

WDate WDate::fromJulianDay(int jd)
{
  int julian = jd;
  int day, month, year;

  if (julian < 0) {
    julian = 0;
  }

  int a = julian;

  if (julian >= 2299161) {
    int jadj = (int)(((float)(julian - 1867216) - 0.25) / 36524.25);
    a += 1 + jadj - (int)(0.25 * jadj);
  }

  int b = a + 1524;
  int c = (int)(6680.0 + ((float)(b - 2439870) - 122.1) / 365.25);
  int d = (int)(365 * c + (0.25 * c));
  int e = (int)((b - d) / 30.6001);

  day = b - d - (int)(30.6001 * e);
  month = e - 1;

  if (month > 12) {
    month -= 12;
  }

  year = c - 4715;

  if (month > 2) {
    --year;
  }

  if (year <= 0) {
    --year;
  }

  return WDate(year, month, day);
}

static void fatalFormatError(const WString& format, int c, const char* cs)
{
  std::stringstream s;
  s << "WDate format syntax error (for \"" << format.toUTF8()
    << "\"): Cannot handle " << c << " consecutive " << cs;

  throw WException(s.str());
}

bool WDate::parseLast(const std::string& v, unsigned& vi,
		      ParseState& parse,
		      const WString& format)
{
  if (parse.d != 0) {
    switch (parse.d) {
    case 1: {
      std::string dstr;

      if (vi >= v.length())
	return false;
      dstr += v[vi++];

      if (vi < v.length())
	if ('0' <= v[vi] && v[vi] <= '9')
	  dstr += v[vi++];

      try {
	parse.day = Utils::stoi(dstr);
      } catch (std::exception&) {
	return false;
      }
      
      break;
    }
    case 2: {
      if (vi + 1 >= v.length())
	return false;

      std::string dstr = v.substr(vi, 2);
      vi += 2;

      try {
	parse.day = Utils::stoi(dstr);
      } catch (std::exception&) {
	return false;
      }      

      break;
    }
    case 3:
      if (parseShortDayName(v, vi) == -1)
	return false;
      break;
    case 4:
      if (parseLongDayName(v, vi) == -1)
	return false;
      break;
    default:
      fatalFormatError(format, parse.d, "d's");
    }

    parse.d = 0;
  }

  if (parse.M != 0) {
    switch (parse.M) {
    case 1: {
      std::string Mstr;

      if (vi >= v.length())
	return false;
      Mstr += v[vi++];

      if (vi < v.length())
	if ('0' <= v[vi] && v[vi] <= '9')
	  Mstr += v[vi++];

      try {
	parse.month = Utils::stoi(Mstr);
      } catch (std::exception&) {
	return false;
      }
      
      break;
    }
    case 2: {
      if (vi + 1 >= v.length())
	return false;

      std::string Mstr = v.substr(vi, 2);
      vi += 2;

      try {
	parse.month = Utils::stoi(Mstr);
      } catch (std::exception&) {
	return false;
      }      

      break;
    }
    case 3:
      parse.month = parseShortMonthName(v, vi);
      if (parse.month == -1)
	return false;
      break;
    case 4:
      parse.month = parseLongMonthName(v, vi);
      if (parse.month == -1)
	return false;
      break;
    default:
      fatalFormatError(format, parse.M, "M's");
    }

    parse.M = 0;
  }

  if (parse.y != 0) {
    switch (parse.y) {
    case 2: {
      if (vi + 1 >= v.length())
	return false;

      std::string ystr = v.substr(vi, 2);
      vi += 2;

      try {
	parse.year = Utils::stoi(ystr);
	if (parse.year < 38)
	  parse.year += 2000;
	else
	  parse.year += 1900;
      } catch (std::exception&) {
	return false;
      }      

      break;
    }
    case 4: {
      if (vi + 3 >= v.length())
	return false;

      std::string ystr = v.substr(vi, 4);
      vi += 4;

      try {
	parse.year = Utils::stoi(ystr);
      } catch (std::exception&) {
	return false;
      }      

      break;
    }
    default:
      fatalFormatError(format, parse.y, "y's");
    }

    parse.y = 0;
  }

  return true;
}

WString WDate::toString() const
{
  return WDate::toString(defaultFormat());
}

WString WDate::toString(const WString& format) const
{
  return WDateTime::toString(this, nullptr, format, true, 0);
}

bool WDate::writeSpecial(const std::string& f, unsigned& i,
			 WStringStream& result, bool localized) const
{
  char buf[30];

  switch (f[i]) {
  case 'd':
    if (f[i + 1] == 'd') {
      if (f[i + 2] == 'd') {
	if (f[i + 3] == 'd') {
	  // 4 d's
	  i += 3;
	  result << longDayName(dayOfWeek(), localized).toUTF8();
	} else {
	  // 3 d's
	  i += 2;
	  result << shortDayName(dayOfWeek(), localized).toUTF8();
	}
      } else {
	// 2 d's
	i += 1;
	result << Utils::pad_itoa(day(), 2, buf);
      }
    } else {
      // 1 d
      result << Utils::itoa(day(), buf);
    }

    return true;
  case 'M':
    if (f[i + 1] == 'M') {
      if (f[i + 2] == 'M') {
	if (f[i + 3] == 'M') {
	  // 4 M's
	  i += 3;
	  result << longMonthName(month(), localized).toUTF8();
	} else {
	  // 3 M's
	  i += 2;
	  result << shortMonthName(month(), localized).toUTF8();
	}
      } else {
	// 2 M's
	i += 1;
	result << Utils::pad_itoa(month(), 2, buf);
      }
    } else {
      // 1 M
      result << Utils::itoa(month(), buf);
    }

    return true;
  case 'y':
    if (f[i + 1] == 'y') {
      if (f[i + 2] == 'y' && f[i + 3] == 'y') {
	// 4 y's
	i += 3;
	result << Utils::itoa(year(), buf);

	return true;
      } else {
	// 2 y's
	i += 1;
	result << Utils::pad_itoa(year() % 100, 2, buf);

	return true;
      }
    }
  default:
    return false;
  }
}

namespace {

  std::string extLiteral(char c) {
    std::string result("");
    switch (c) {
      case ',': case 'D': case 'j': case 'l':
      case 'S': case 'w': case 'z': case 'W':
      case 'F': case 'm': case 'M': case 'n':
      case 't': case 'L': case 'Y': case 'y':
      case 'a': case 'A': case 'g': case 'G':
      case 'h': case 'H': case 'i': case 's':
      case 'O': case 'T': case 'Z': case 0:
      result.push_back('\\');
    }
    result.push_back(c);
    return result;
  }

  void writeExtLast(std::string& result, int& d, int& M, int& y,
		    const WString& format) {
    if (d != 0) {
      switch (d) {
      case 1:
	result += 'j'; break;
      case 2:
	result += 'd'; break;
      case 3:
	result += 'D'; break;
      case 4:
	result += 'l'; break;
      default:
	fatalFormatError(format, d, "d's");
      }

      d = 0;
    }

    if (M != 0) {
      switch (M) {
      case 1:
	result += 'n'; break;
      case 2:
	result += 'm'; break;
      case 3:
	result += 'M'; break;
      case 4:
	result += 'F'; break;
      default:
	fatalFormatError(format, M, "M's");
      }

      M = 0;
    }

    if (y != 0) {
      switch (y) {
      case 2:
	result += 'y'; break;
      case 4:
	result += 'Y'; break;
      default:
	fatalFormatError(format, y, "y's");
      }

      y = 0;
    }
  }
}

std::string WDate::extFormat(const WString& format)
{
  std::string result;
  std::string f = format.toUTF8();

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  int d = 0, M = 0, y = 0; 

  for (unsigned i = 0; i < f.length(); ++i) {
    if (inQuote) {
      if (f[i] != '\'') {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else
	  result += extLiteral(f[i]);
      } else {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  result += extLiteral(f[i]);
	} else
	  gotQuoteInQuote = true;
      }
    }

    if (!inQuote) {
      switch (f[i]) {
      case 'd':
	if (d == 0)
	  writeExtLast(result, d, M, y, format);
	++d;
	break;
      case 'M':
	if (M == 0)
	  writeExtLast(result, d, M, y, format);
	++M;
	break;
      case 'y':
	if (y == 0)
	  writeExtLast(result, d, M, y, format);
	++y;
	break;
      default:
	writeExtLast(result, d, M, y, format);
	if (f[i] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
	  result += extLiteral(f[i]);
      }
    }
  }

  writeExtLast(result, d, M, y, format);

  return result;
}

namespace {

  void fatalFormatRegExpError(const WString& format, int c, const char* cs)
  {
    std::stringstream s;
    s << "WDate to regexp: (for \"" << format.toUTF8()
      << "\"): cannot handle " << c << " consecutive " << cs;
    throw WException(s.str());
  }

  void writeRegExpLast(WDate::RegExpInfo& result, int& d, int& M, int& y,
		       const WString& format, int& currentGroup) {
    if (d != 0) {
      switch (d) {
      case 1:
      case 2:
	if (d == 1)
	  result.regexp += "(\\d{1,2})";
	else
	  result.regexp += "(\\d{2})";

	result.dayGetJS = "return parseInt(results["
	  + std::to_string(currentGroup++) + "], 10);";
	break;
      default:
	fatalFormatRegExpError(format, d, "d's");
      }

      d = 0;
    }

    if (M != 0) {
      switch (M) {
      case 1:
      case 2:
	if (M == 1)
	  result.regexp += "(\\d{1,2})";
	else
	  result.regexp += "(\\d{2})";

	result.monthGetJS = "return parseInt(results["
	  + std::to_string(currentGroup++) + "], 10);";
	break;
      default:
	fatalFormatRegExpError(format, M, "M's");
      }

      M = 0;
    }

    if (y != 0) {
      switch (y) {
      case 2:
	result.regexp += "(\\d{2})";
	result.yearGetJS = "var y=parseInt(results["
	  + std::to_string(currentGroup++) + "], 10);"
	  "return y > 38 ? 1900 + y : 2000 + y;";
	break;
      case 4:
	result.regexp += "(\\d{4})";
	result.yearGetJS = "return parseInt(results["
	  + std::to_string(currentGroup++) + "], 10)";
	break;
      default:
	fatalFormatRegExpError(format, y, "y's");
      }

      y = 0;
    }
  }
}

WDate::RegExpInfo WDate::formatToRegExp(const WT_USTRING& format)
{
  RegExpInfo result;
  std::string f = format.toUTF8();
  int currentGroup = 1;

  result.dayGetJS = "return 1";
  result.monthGetJS = "return 1";
  result.yearGetJS = "return 2000";

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  static const std::string regexSpecial = "/[\\^$.|?*+()";

  int d = 0, M = 0, y = 0; 

  for (unsigned i = 0; i < f.length(); ++i) {
    if (inQuote) {
      if (f[i] != '\'') {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else
	  result.regexp += f[i];
      } else {
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  result.regexp += f[i];
	} else
	  gotQuoteInQuote = true;
      }
    }

    if (!inQuote) {
      switch (f[i]) {
      case 'd':
	if (d == 0)
	  writeRegExpLast(result, d, M, y, format, currentGroup);
	++d;
	break;
      case 'M':
	if (M == 0)
	  writeRegExpLast(result, d, M, y, format, currentGroup);
	++M;
	break;
      case 'y':
	if (y == 0)
	  writeRegExpLast(result, d, M, y, format, currentGroup);
	++y;
	break;
      default:
	writeRegExpLast(result, d, M, y, format, currentGroup);
	if (f[i] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else if (regexSpecial.find(f[i]) != std::string::npos) {
	  result.regexp += "\\";
	  result.regexp += f[i];
	} else
	  result.regexp += f[i];
      }
    }
  }

  writeRegExpLast(result, d, M, y, format, currentGroup);

  return result;
}

}
