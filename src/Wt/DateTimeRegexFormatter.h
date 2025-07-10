// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef DATE_TIME_REGEX_FORMATTER_
#define DATE_TIME_REGEX_FORMATTER_

#include "Wt/WString.h"
#include "Wt/WLogger.h"

#ifdef WT_TARGET_JAVA
  #define WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START static
#else
  #define WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START template<typename RegexInfo>
#endif

namespace Wt {

  namespace Utils {

#ifdef WT_TARGET_JAVA

  struct RegexInfo {
    std::string regexp;
    std::string yearGetJS;
    std::string monthGetJS;
    std::string dayGetJS;
    std::string hourGetJS;
    std::string minuteGetJS;
    std::string secGetJS;
    std::string msecGetJS;
    int index;
    int currentGroup;
  };

class DateTimeRegexFormatter
{
public:
#endif
  static const std::string specialRegex = ".+$^*[]{}()?!/\\|";

#ifdef WT_TARGET_JAVA
  static
#endif
  void fatalFormatRegExpError(const WString& format, int c, const char* cs);

#ifndef WT_TARGET_JAVA
  bool hasAP(const std::string& format);
#endif

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo processChar(RegexInfo& info, const std::string& format)
  {
    if (specialRegex.find(format[info.index]) != std::string::npos) {
      info.regexp += "\\";
    }

    info.regexp += format[info.index];
    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatYearToRegex(RegexInfo& info, const std::string& format)
  {
    char next = 0;
    std::string sf;
    sf += format[info.index];

    while (info.index < format.size() - 1) {
      next = format[info.index + 1];
      if (next != 'y') {
        break;
      }

      sf+= next;
      ++info.index;
    }

    if (sf == "yy") {
      // year expressed in two last number
      info.regexp += "(\\d{2})";
      info.yearGetJS = "var y = parseInt(results["
        + std::to_string(info.currentGroup++) + "], 10);"
        " return y > 38 ? 1900 + y : 2000 + y;";
    } else if (sf == "yyyy") {
      // year expressed in full
      info.regexp += "(\\d{4})";
      info.yearGetJS = "return parseInt(results["
        + std::to_string(info.currentGroup++) + "], 10);";
    } else {
      fatalFormatRegExpError(format, sf.size(), "y");
    }

    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatMonthToRegex(RegexInfo& info, const std::string& format)
  {
    char next = 0;
    std::string sf;
    sf += format[info.index];

    while (info.index < format.size() - 1) {
      next = format[info.index + 1];
      if (next != 'M') {
        break;
      }

      sf+= next;
      ++info.index;
    }

    if (sf == "M") {
      // month without leading 0
      info.regexp += "(\\d{1,2})";
    } else if (sf == "MM") {
      // month potentially with leading 0
      info.regexp += "(\\d{2})";
    } else {
      fatalFormatRegExpError(format, sf.size(), "M");
    }

    info.monthGetJS = "return parseInt(results["
      + std::to_string(info.currentGroup++) + "], 10);";

    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatDayToRegex(RegexInfo& info, const std::string& format)
  {
    char next = 0;
    std::string sf;
    sf += format[info.index];

    while (info.index < format.size() - 1) {
      next = format[info.index + 1];
      if (next != 'd') {
        break;
      }

      sf+= next;
      ++info.index;
    }

    if (sf == "d") {
      // day without leading 0
      info.regexp += "(\\d{1,2})";
    } else if (sf == "dd") {
      // day potentially with leading 0
      info.regexp += "(\\d{2})";
    } else {
      fatalFormatRegExpError(format, sf.size(), "d");
    }

    info.dayGetJS = "return parseInt(results["
      + std::to_string(info.currentGroup++) + "], 10);";

    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatHourToRegex(RegexInfo& info, const std::string& format)
  {
    char next = 0;

#ifndef WT_TARGET_JAVA
    bool ap = hasAP(format);
#else
    bool ap = true;
#endif
    std::string sf;
    sf += format[info.index];

    if (info.index < format.size() - 1)
      next = format[info.index + 1];

    if (next == 'h' || next == 'H') {
      sf+= next;
      ++info.index;
    }

    if (sf == "HH" || (sf == "hh" && !ap)) {
      // hour with leading 0, ranging 0-23
      info.regexp += "([0-1][0-9]|2[0-3])";
    } else if (sf == "hh" && ap) {
      // hour with leading 0, ranging 01-12
      info.regexp += "(0[1-9]|1[012])";
    } else if (sf == "H" || (sf == "h" && !ap)) {
      // hour without leading 0, ranging 0-23
      info.regexp += "(0|[1-9]|1[0-9]|2[0-3])";
    } else if (sf == "h" && ap) {
      // hour without leading 0, ranging 1-12
      info.regexp += "([1-9]|1[012])";
    } else {
      fatalFormatRegExpError(format, sf.size(), "h/H");
    }

    info.hourGetJS = "return parseInt(results[" +
      std::to_string(info.currentGroup++) + "], 10);";

    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatMinuteToRegex(RegexInfo& info, const std::string& format)
  {
    char next = 0;
    std::string sf;
    sf += format[info.index];

    if (info.index < format.size() - 1) {
      next = format[info.index + 1];
    }

    if (next == 'm') {
      sf += next;
      ++info.index;
    }

    if (sf == "m") {
      // minutes without leading 0
      info.regexp += "(0|[1-5]?[0-9])";
    } else if (sf == "mm") {
      // minutes potentially with leading 0
      info.regexp += "([0-5][0-9])";
    } else {
      fatalFormatRegExpError(format, sf.size(), "m");
    }

    info.minuteGetJS = "return parseInt(results["
            + std::to_string(info.currentGroup++) + "], 10);";

    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatSecondToRegex(RegexInfo& info, const std::string& format)
  {
    char next = 0;
    std::string sf;
    sf += format[info.index];

    if (info.index < format.size() - 1) {
      next = format[info.index + 1];
    }

    if (next == 's') {
      sf += next;
      ++info.index;
    }

    if (sf == "s") {
      // seconds without leading 0
      info.regexp += "(0|[1-5]?[0-9])";
    } else if (sf == "ss") {
      // seconds potentially with leading 0
      info.regexp += "([0-5][0-9])";
    } else {
      fatalFormatRegExpError(format, sf.size(), "s");
    }

    info.secGetJS = "return parseInt(results["
            + std::to_string(info.currentGroup++) + "], 10);";

    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatAPToRegex(RegexInfo& info, const std::string& format)
  {
#ifndef WT_TARGET_JAVA
    char next = 0;
    std::string sf;
    sf += format[info.index];

    if (info.index < format.size() - 1) {
      next = format[info.index + 1];
    }

    if (next == 'P' || next == 'p') {
      sf += next;
      ++info.index;
    }

    if (sf == "AP" || sf == "A") {
#endif

      info.regexp += "([AP]M)";

#ifndef WT_TARGET_JAVA
    } else if (sf == "ap" || sf == "a") {
      info.regexp += "([ap]m)";
    } else {
      // Inconsistency: but breaking behaviour.
      // Currently this will ignore e.g. "APA", but produce no error.
      // fatalFormatRegExpError(format, sf.size(), "AP/AM/A/P");
      info.regexp += format[info.index];
    }
#endif

    ++info.currentGroup;
    return info;
  }

  WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START
  RegexInfo formatMSecondToRegex(RegexInfo& info, const std::string& format)
  {

#ifndef WT_TARGET_JAVA
    constexpr char msecChar = 'z';
#else
    char msecChar = 'S';
#endif

    char next = 0;
    std::string sf;
    sf += format[info.index];

    for (int k = 0; k < 2; ++k) {
      if (info.index < format.size() - 1) {
        next = format[info.index + 1];
      }

      if (next == msecChar) {
        sf += msecChar;
        next = 0;
        ++info.index;
      } else {
        next = 0;
        break;
      }
    }

    if (sf.length() == 1) {
      // milliseconds without trailing 0
      info.regexp += "(0|[1-9][0-9]{0,2})";
    } else if (sf.length() == 3) {
      // milliseconds potantially with trailing 0
      info.regexp += "([0-9]{3})";
    }

    // Inconsistency: but breaking behaviour.
    // Currently this will ignore e.g. "zz", but produce no error.
    // This makes 'H:m:s.zz' equivalent to 'H:m:s.'.
    /*} else {
      fatalFormatRegExpError(format, sf.size(), "z");
    }*/

    info.msecGetJS = "return parseInt(results["
                      + std::to_string(info.currentGroup++) + "], 10);";

    return info;
  }

#ifdef WT_TARGET_JAVA
};
#endif

  }
}

#undef WT_DATE_TIME_REGEX_FORMATTER_FUNCTION_START

#endif // DATE_TIME_REGEX_FORMATTER_
