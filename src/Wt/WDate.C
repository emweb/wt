/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdlib.h>

#include "Wt/WApplication"
#include "Wt/WDate"
#include "Wt/WLogger"

#include "WtException.h"
#include "Utils.h"

#include <boost/date_time/gregorian/gregorian.hpp>
using namespace boost::gregorian;

namespace {
  std::string WT_WDATE = "Wt.WDate.";
}

namespace Wt {

InvalidDateException::InvalidDateException()
{ }

InvalidDateException::~InvalidDateException() throw()
{ }

const char *InvalidDateException::what() const throw()
{ 
  return "Error: Attempted operation on an invalid WDate";
}

WDate::WDate()
  : valid_(false),
    year_(-1),
    month_(0),
    day_(0)
{ }

WDate::WDate(int year, int month, int day)
{
  setDate(year, month, day);
}

WDate WDate::addDays(int ndays) const
{
  if (valid_) {
    date d(year_, month_, day_);
    d += date_duration(ndays);

    return WDate(d.year(), d.month(), d.day());
  } else
    return *this;
}

WDate WDate::addMonths(int nmonths) const
{
  if (valid_) {
    int month = month_ + nmonths;
    div_t a = div(month - 1, 12);
    int year = year_ + a.quot;
    month = 1 + a.rem;

    if (month <= 0) {
      month += 12;
      year -= 1;
    }

    return WDate(year, month, day_);
  } else
    return *this;
}

WDate WDate::addYears(int nyears) const
{
  if (valid_) {
    return WDate(year_ + nyears, month_, day_);
  } else
    return *this;
}

void WDate::setDate(int year, int month, int day)
{
  year_ = year;
  month_ = month;
  day_ = day;
  valid_ = false;

  try {
    date d(year, month, day);
    valid_ = true;
  } catch (std::out_of_range& e) {
    WApplication *app = wApp;
    if (app)
      app->log("warn") << "Invalid date: " << e.what();
  }
}

int WDate::dayOfWeek() const
{
  if (!isValid())
    throw InvalidDateException();

  date d(year_, month_, day_);

  int dow = d.day_of_week().as_number();

  return (dow == 0 ? 7 : dow);
}

int WDate::daysTo(const WDate& other) const
{
  if (!isValid() || !other.isValid())
    throw InvalidDateException();

  date dthis(year_, month_, day_);
  date dother(other.year_, other.month_, other.day_);
  date_duration dd = dother - dthis;

  return dd.days();
}

int WDate::toJulianDay() const
{
  if (!isValid())
    return -1;
  else {
    date dthis(year_, month_, day_);
    return dthis.julian_day();
  }
}

bool WDate::isNull() const
{
  return year_ == -1;
}

bool WDate::isLeapYear(int year)
{
  return isValid(year, 2, 29);
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
  if (!isValid() || !other.isValid())
    throw InvalidDateException();

  if (year_ < other.year_)
    return true;
  else
    if (year_ == other.year_) {
      if (month_ < other.month_)
	return true;
      else
	if (month_ == other.month_)
	  return day_ < other.day_;
	else
	  return false;
    } else
      return false;
}

bool WDate::operator!= (const WDate& other) const
{
  return !(*this == other);
}

bool WDate::operator== (const WDate& other) const
{
  if ((!isValid() && !isNull()) || (!other.isValid() && !other.isNull()))
    throw InvalidDateException();

  return (year_ == other.year_ && month_ == other.month_ && day_ == other.day_);
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
  date cd = day_clock::local_day();

  return WDate(cd.year(), cd.month(), cd.day());
}

WDate WDate::currentDate()
{
  return currentServerDate(); // FIXME
}

WString WDate::shortDayName(int weekday)
{
  static const char *v[]
    = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

  if (WApplication::instance())
    return WString::tr(WT_WDATE + v[weekday - 1]);
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

WString WDate::longDayName(int weekday)
{
  static const char *v[]
    = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
       "Sunday" };

  if (WApplication::instance())
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

WString WDate::shortMonthName(int month)
{
  static const char *v[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

  if (WApplication::instance())
    return WString::tr(WT_WDATE + v[month - 1]);
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

WString WDate::longMonthName(int month)
{
  static const char *v[] = {"January", "February", "March", "April", "May",
			    "June", "July", "August", "September",
			    "October", "November", "December" };

  if (WApplication::instance())
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

  WDateTime::fromString(&result, 0, s, format);

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
	return WDateTime::CharInvalid;

    ++parse.d;

    return WDateTime::CharHandled;

  case 'M':
    if (parse.M == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharInvalid;

    ++parse.M;

    return WDateTime::CharHandled;

  case 'y':
    if (parse.y == 0)
      if (!parseLast(v, vi, parse, format))
	return WDateTime::CharInvalid;

    ++parse.y;

    return WDateTime::CharHandled;

  default:
    if (!parseLast(v, vi, parse, format))
      return WDateTime::CharInvalid;

    return WDateTime::CharUnhandled;
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

  throw WtException(s.str());
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
	parse.day = boost::lexical_cast<int>(dstr);
      } catch (boost::bad_lexical_cast&) {
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
	parse.day = boost::lexical_cast<int>(dstr);
      } catch (boost::bad_lexical_cast&) {
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
	parse.month = boost::lexical_cast<int>(Mstr);
      } catch (boost::bad_lexical_cast&) {
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
	parse.month = boost::lexical_cast<int>(Mstr);
      } catch (boost::bad_lexical_cast&) {
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
	parse.year = boost::lexical_cast<int>(ystr);
	if (parse.year < 38)
	  parse.year += 2000;
	else
	  parse.year += 1900;
      } catch (boost::bad_lexical_cast&) {
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
	parse.year = boost::lexical_cast<int>(ystr);
      } catch (boost::bad_lexical_cast&) {
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
  return WDateTime::toString(this, 0, format);
}

bool WDate::writeSpecial(const std::string& f, unsigned&i,
			 std::stringstream& result) const
{
  char buf[30];

  switch (f[i]) {
  case 'd':
    if (f[i + 1] == 'd') {
      if (f[i + 2] == 'd') {
	if (f[i + 3] == 'd') {
	  // 4 d's
	  i += 3;
	  result << longDayName(dayOfWeek()).toUTF8();
	} else {
	  // 3 d's
	  i += 2;
	  result << shortDayName(dayOfWeek()).toUTF8();
	}
      } else {
	// 2 d's
	i += 1;
	result << Utils::pad_itoa(day_, 2, buf);
      }
    } else {
      // 1 d
      result << Utils::itoa(day_, buf);
    }

    return true;
  case 'M':
    if (f[i + 1] == 'M') {
      if (f[i + 2] == 'M') {
	if (f[i + 3] == 'M') {
	  // 4 M's
	  i += 3;
	  result << longMonthName(month_).toUTF8();
	} else {
	  // 3 M's
	  i += 2;
	  result << shortMonthName(month_).toUTF8();
	}
      } else {
	// 2 M's
	i += 1;
	result << Utils::pad_itoa(month_, 2, buf);
      }
    } else {
      // 1 M
      result << Utils::itoa(month_, buf);
    }

    return true;
  case 'y':
    if (f[i + 1] == 'y') {
      if (f[i + 2] == 'y' && f[i + 3] == 'y') {
	// 4 y's
	i += 3;
	result << Utils::itoa(year_, buf);

	return true;
      } else {
	// 2 y's
	i += 1;
	result << Utils::pad_itoa(year_ % 100, 2, buf);

	return true;
      }
    }
  default:
    return false;
  }
}

namespace {

  std::string extLiteral(char c) {
    char extSpecial[] = { ',', 'D', 'j', 'l', 'S', 'w', 'z', 'W', 'F', 'm', 'M',
			  'n', 't', 'L', 'Y', 'y', 'a', 'A', 'g', 'G', 'h', 'H',
			  'i', 's', 'O', 'T', 'Z', 0 };

    for (int i = 0; extSpecial[i]; ++i) {
      if (c == extSpecial[i])
	return "\\" + c;
    }

    return std::string() + c;
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
    throw WtException(s.str());
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

	result.dayGetJS = "parseInt(results["
	  + boost::lexical_cast<std::string>(currentGroup++) + "], 10)";
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

	result.monthGetJS = "parseInt(results["
	  + boost::lexical_cast<std::string>(currentGroup++) + "], 10)";
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
	result.yearGetJS = "function() { var y=parseInt(results["
	  + boost::lexical_cast<std::string>(currentGroup++) + "], 10);"
	  "return y>38?1900+y:2000+y;}()";
	break;
      case 4:
	result.regexp += "(\\d{4})";
	result.yearGetJS = "parseInt(results["
	  + boost::lexical_cast<std::string>(currentGroup++) + "], 10)";
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

  result.dayGetJS = "1";
  result.monthGetJS = "1";
  result.yearGetJS = "2000";

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
