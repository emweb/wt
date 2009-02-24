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

#include <boost/date_time/gregorian/gregorian.hpp>
using namespace boost::gregorian;

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

WDate::WDate(long modifiedJulianDays)
{
  date d(1858, 11, 17);
  d += date_duration(modifiedJulianDays);
  setDate(d.year(), d.month(), d.day());
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
    wApp->log("warn") << "Invalid date: " << e.what();
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

long WDate::modifiedJulianDay() const
{
  if (!isValid())
    return -1;
  else {
    date dthis(year_, month_, day_);
    return dthis.modjulian_day();
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
  if (!isValid() || !other.isValid())
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
  static const char *v[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

  return WString(v[weekday - 1], UTF8);
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
    = {"Monday", "Tuesday", "Wednesday", "Thursday",
       "Friday", "Saturday", "Sunday" };

  return WString(v[weekday - 1], UTF8);
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

  return WString(v[month - 1], UTF8);
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
		      "June", "July", "August", "September", "October",
		      "November", "December" };

  return WString(v[month - 1], UTF8);
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
  return WString("ddd MMM d yyyy", UTF8); 
}

WDate WDate::fromString(const WString& s)
{
  return fromString(s, defaultFormat());
}

WDate WDate::fromString(const WString& s, const WString& format)
{
  std::string v = s.toUTF8();
  std::string f = format.toUTF8();

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  unsigned vi = 0;

  int d = 0, M = 0, y = 0; 
  int day = -1, month = -1, year = -1;

  for (unsigned fi = 0; fi < f.length(); ++fi) {
    if (inQuote)
      if (f[fi] != '\'')
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else {
	  if (vi >= v.length() || (v[vi++] != f[fi]))
	    return WDate();
	}
      else
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  if (vi >= v.length() || (v[vi++] != f[fi]))
	    return WDate();
	} else
	  gotQuoteInQuote = true;

    if (!inQuote) {
      switch (f[fi]) {
      case 'd':
	if (d == 0)
	  if (!parseLast(v, vi, d, M, y, day, month, year, format))
	    return WDate();
	++d;
	break;
      case 'M':
	if (M == 0)
	  if (!parseLast(v, vi, d, M, y, day, month, year, format))
	    return WDate();
	++M;
	break;
      case 'y':
	if (y == 0)
	  if (!parseLast(v, vi, d, M, y, day, month, year, format))
	    return WDate();
	++y;
	break;
      default:
	if (!parseLast(v, vi, d, M, y, day, month, year, format))
	  return WDate();

	if (f[fi] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
	  if (vi >= v.length() || (v[vi++] != f[fi]))
	    return WDate();
      }
    }
  }

  if (!parseLast(v, vi, d, M, y, day, month, year, format))
    return WDate();

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
		      int& d, int& M, int& y,
		      int& day, int& month, int& year,
		      const WString& format)
{
  if (d != 0) {
    switch (d) {
    case 1: {
      std::string dstr;

      if (vi >= v.length())
	return false;
      dstr += v[vi++];

      if (vi < v.length())
	if ('0' <= v[vi] && v[vi] <= '9')
	  dstr += v[vi++];

      try {
	day = boost::lexical_cast<int>(dstr);
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
	day = boost::lexical_cast<int>(dstr);
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
      fatalFormatError(format, d, "d's");
    }

    d = 0;
  }

  if (M != 0) {
    switch (M) {
    case 1: {
      std::string Mstr;

      if (vi >= v.length())
	return false;
      Mstr += v[vi++];

      if (vi < v.length())
	if ('0' <= v[vi] && v[vi] <= '9')
	  Mstr += v[vi++];

      try {
	month = boost::lexical_cast<int>(Mstr);
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
	month = boost::lexical_cast<int>(Mstr);
      } catch (boost::bad_lexical_cast&) {
	return false;
      }      

      break;
    }
    case 3:
      month = parseShortMonthName(v, vi);
      if (month == -1)
	return false;
      break;
    case 4:
      month = parseLongMonthName(v, vi);
      if (month == -1)
	return false;
      break;
    default:
      fatalFormatError(format, M, "M's");
    }

    M = 0;
  }

  if (y != 0) {
    switch (y) {
    case 2: {
      if (vi + 1 >= v.length())
	return false;

      std::string ystr = v.substr(vi, 2);
      vi += 2;

      try {
	year = boost::lexical_cast<int>(ystr);
	if (year < 38)
	  year += 2000;
	else
	  year += 1900;
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
	year = boost::lexical_cast<int>(ystr);
      } catch (boost::bad_lexical_cast&) {
	return false;
      }      

      break;
    }
    default:
      fatalFormatError(format, y, "y's");
    }

    y = 0;
  }

  return true;
}

WString WDate::toString() const
{
  return WDate::toString(defaultFormat());
}

WString WDate::toString(const WString& format) const
{
  if (!isValid())
    return WString("Null", UTF8);

  std::string result;
  std::string f = format.toUTF8();

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  int d = 0, M = 0, y = 0; 

  for (unsigned i = 0; i < f.length(); ++i) {
    if (inQuote)
      if (f[i] != '\'')
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else
	  result += f[i];
      else
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  result += f[i];
	} else
	  gotQuoteInQuote = true;

    if (!inQuote) {
      switch (f[i]) {
      case 'd':
	if (d == 0)
	  writeLast(result, d, M, y, format);
	++d;
	break;
      case 'M':
	if (M == 0)
	  writeLast(result, d, M, y, format);
	++M;
	break;
      case 'y':
	if (y == 0)
	  writeLast(result, d, M, y, format);
	++y;
	break;
      default:
	writeLast(result, d, M, y, format);
	if (f[i] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
	  result += f[i];
      }
    }
  }

  writeLast(result, d, M, y, format);

  return WString::fromUTF8(result);
}

void WDate::writeLast(std::string& result, int& d, int& M, int& y,
		      const WString& format) const
{
  if (d != 0) {
    switch (d) {
    case 1:
      result += boost::lexical_cast<std::string>(day_);
      break;
    case 2:
      if (day_ < 10)
	result += "0";
      result += boost::lexical_cast<std::string>(day_);
      break;
    case 3:
      result += shortDayName(dayOfWeek()).toUTF8();
      break;
    case 4:
      result += longDayName(dayOfWeek()).toUTF8();
      break;
    default:
      fatalFormatError(format, d, "d's");
    }

    d = 0;
  }

  if (M != 0) {
    switch (M) {
    case 1:
      result += boost::lexical_cast<std::string>(month_);
      break;
    case 2:
      if (month_ < 10)
	result += "0";
      result += boost::lexical_cast<std::string>(month_);
      break;
    case 3:
      result += shortMonthName(month_).toUTF8();
      break;
    case 4:
      result += longMonthName(month_).toUTF8();
      break;
    default:
      fatalFormatError(format, M, "M's");
    }

    M = 0;
  }

  if (y != 0) {
    switch (y) {
    case 2: {
      int yy = year_ % 100;
      if (yy < 10)
	result += '0';
      result += boost::lexical_cast<std::string>(yy);
      break;
    }
    case 4:
      result += boost::lexical_cast<std::string>(year_);
      break;
    default:
      fatalFormatError(format, y, "y's");
    }

    y = 0;
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
    if (inQuote)
      if (f[i] != '\'')
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else
	  result += extLiteral(f[i]);
      else
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  result += extLiteral(f[i]);
	} else
	  gotQuoteInQuote = true;

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

  void writeRegExpLast(std::string& result, int& d, int& M, int& y,
		       const WString& format, int& currentGroup,
		       std::string& dg, std::string& mg, std::string& yg) {
    if (d != 0) {
      switch (d) {
      case 1:
      case 2:
	if (d == 1)
	  result += "(\\d{1,2})";
	else
	  result += "(\\d{2})";
	dg = "parseInt(results["
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
	  result += "(\\d{1,2})";
	else
	  result += "(\\d{2})";
	mg = "parseInt(results["
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
	result += "(\\d{2})";
	yg = "function() { var y=parseInt(results["
	  + boost::lexical_cast<std::string>(currentGroup++) + "], 10);"
	  "return y>38?1900+y:2000+y;}()";
	break;
      case 4:
	result += "(\\d{4})";
	yg = "parseInt(results["
	  + boost::lexical_cast<std::string>(currentGroup++) + "], 10)";
	break;
      default:
	fatalFormatRegExpError(format, y, "y's");
      }

      y = 0;
    }
  }
}

std::string WDate::formatToRegExp(const WString& format,
				  std::string& dayGetJS,
				  std::string& monthGetJS,
				  std::string& yearGetJS)
{
  std::string result;
  std::string f = format.toUTF8();
  int currentGroup = 1;

  bool inQuote = false;
  bool gotQuoteInQuote = false;

  int d = 0, M = 0, y = 0; 

  for (unsigned i = 0; i < f.length(); ++i) {
    if (inQuote)
      if (f[i] != '\'')
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  inQuote = false;
	} else
	  result += f[i];
      else
	if (gotQuoteInQuote) {
	  gotQuoteInQuote = false;
	  result += f[i];
	} else
	  gotQuoteInQuote = true;

    if (!inQuote) {
      switch (f[i]) {
      case 'd':
	if (d == 0)
	  writeRegExpLast(result, d, M, y, format,
			  currentGroup, dayGetJS, monthGetJS, yearGetJS);
	++d;
	break;
      case 'M':
	if (M == 0)
	  writeRegExpLast(result, d, M, y, format,
			  currentGroup, dayGetJS, monthGetJS, yearGetJS);
	++M;
	break;
      case 'y':
	if (y == 0)
	  writeRegExpLast(result, d, M, y, format,
			  currentGroup, dayGetJS, monthGetJS, yearGetJS);
	++y;
	break;
      default:
	writeRegExpLast(result, d, M, y, format,
			currentGroup, dayGetJS, monthGetJS, yearGetJS);
	if (f[i] == '\'') {
	  inQuote = true;
	  gotQuoteInQuote = false;
	} else
	  result += f[i];
      }
    }
  }

  writeRegExpLast(result, d, M, y, format,
		  currentGroup, dayGetJS, monthGetJS, yearGetJS);

  return result;
}

}
