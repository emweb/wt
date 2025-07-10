/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WException.h"
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>
#include <Wt/Test/WTestEnvironment.h>

#include <web/FileUtils.h>

namespace {

  void setup(Wt::WApplication& app)
  {
    std::string file = app.appRoot() + "types/days_months_names";
    BOOST_REQUIRE(Wt::FileUtils::exists(file + ".xml"));

    app.messageResourceBundle().use(file);
  }

}

BOOST_AUTO_TEST_CASE( WDateTime_toString_non_localized )
{
  // Regression test for issue #11852
  // This produces a string that disregards the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDateTime datetime(Wt::WDate(2022, 1, 2), Wt::WTime(10, 8, 9));
  BOOST_TEST(datetime.toString("hh:mm:ss dd/MM/yyyy", false) == "10:08:09 02/01/2022");
  // January 2nd 2022 is a Sunday
  BOOST_TEST(datetime.toString("ddd MMM", false) == "Sun Jan");
  BOOST_TEST(datetime.toString("dddd MMMM", false) == "Sunday January");
}

BOOST_AUTO_TEST_CASE( WDateTime_toString_localized )
{
  // Regression test for issue #11852
  // This produces a string that disregards the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDateTime datetime(Wt::WDate(2022, 1, 2), Wt::WTime(10, 8, 9));
  BOOST_TEST(datetime.toString() == "Dom Ene 2 10:08:09 2022");
  BOOST_TEST(datetime.toString("hh:mm:ss dd/MM/yyyy") == "10:08:09 02/01/2022");
  // January 2nd 2022 is a Sunday
  BOOST_TEST(datetime.toString("ddd MMM") == "Dom Ene");
  BOOST_TEST(datetime.toString("dddd MMMM") == "Domigo Enero");
  BOOST_TEST(datetime.toString("dddd 'MMMM'") == "Domigo MMMM");
}

BOOST_AUTO_TEST_CASE( WDateTime_fromString_non_localized )
{
  // This parse a string disregarding the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDateTime datetime(Wt::WDate(2022, 1, 2), Wt::WTime(10, 8, 9));
  // Ensure datetime is valid
  BOOST_REQUIRE(datetime != Wt::WDateTime());

  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 02/01/2022", "hh:mm:ss dd/MM/yyyy", false) == datetime));

  // Check that English names are recognized
  // January 2nd 2022 is a Sunday
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Sun 02 Jan 2022", "hh:mm:ss ddd dd MMM yyyy", false) == datetime));
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Sunday 02 January 2022", "hh:mm:ss dddd dd MMMM yyyy", false) == datetime));

  // Check that localized names are not recognized
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Dom 02 Ene 2022", "hh:mm:ss ddd dd MMM yyyy", false) == Wt::WDateTime()));
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Domigo 02 Enero 2022", "hh:mm:ss dddd dd MMMM yyyy", false) == Wt::WDateTime()));
}

BOOST_AUTO_TEST_CASE( WDateTime_fromString_localized )
{
  // This parse a string taking the locale into account
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDateTime datetime(Wt::WDate(2022, 1, 2), Wt::WTime(10, 8, 9));
  // Ensure datetime is valid
  BOOST_REQUIRE(datetime != Wt::WDateTime());

  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 02/01/2022", "hh:mm:ss dd/MM/yyyy") == datetime));

  // Check that localized names are recognized
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Dom 02 Ene 2022", "hh:mm:ss ddd dd MMM yyyy") == datetime));
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Domigo 02 Enero 2022", "hh:mm:ss dddd dd MMMM yyyy") == datetime));

  // Check that English names not recognized
  // January 2nd 2022 is a Sunday
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Sun 02 Jan 2022", "hh:mm:ss ddd dd MMM yyyy") == Wt::WDateTime()));
  BOOST_TEST((Wt::WDateTime::fromString("10:08:09 Sunday 02 January 2022", "hh:mm:ss dddd dd MMMM yyyy") == Wt::WDateTime()));
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_time_date_short)
{
  // Creating the regexp for the validator with time first, then date
  std::string format = "H:m:s.z d-M-yy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "(0|[1-9]|1[0-9]|2[0-3]):(0|[1-5]?[0-9]):(0|[1-5]?[0-9])\\.(0|[1-9][0-9]{0,2}) (\\d{1,2})-(\\d{1,2})-(\\d{2})");
  BOOST_TEST(dateTimeInfo.yearGetJS == "var y = parseInt(results[7], 10); return y > 38 ? 1900 + y : 2000 + y;");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[6], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[5], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return parseInt(results[4], 10);");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_time_date_long)
{
  // Creating the regexp for the validator with time first, then date
  std::string format = "HH:mm:ss.zzz dd-MM-yyyy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])\\.([0-9]{3}) (\\d{2})-(\\d{2})-(\\d{4})");
  BOOST_TEST(dateTimeInfo.yearGetJS == "return parseInt(results[7], 10);");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[6], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[5], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return parseInt(results[4], 10);");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_time_date_both)
{
  // Creating the regexp for the validator with time first, then date
  std::string format = "h:mm:s.zzz ap dd-M-yy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([1-9]|1[012]):([0-5][0-9]):(0|[1-5]?[0-9])\\.([0-9]{3}) ([ap]m) (\\d{2})-(\\d{1,2})-(\\d{2})");
  BOOST_TEST(dateTimeInfo.yearGetJS == "var y = parseInt(results[8], 10); return y > 38 ? 1900 + y : 2000 + y;");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[7], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[6], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return parseInt(results[4], 10);");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_date_time_short)
{
  // Creating the regexp for the validator with date first, then time
  std::string format = "d-M-yy H:m:s.z";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "(\\d{1,2})-(\\d{1,2})-(\\d{2}) (0|[1-9]|1[0-9]|2[0-3]):(0|[1-5]?[0-9]):(0|[1-5]?[0-9])\\.(0|[1-9][0-9]{0,2})");
  BOOST_TEST(dateTimeInfo.yearGetJS == "var y = parseInt(results[3], 10); return y > 38 ? 1900 + y : 2000 + y;");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[4], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[5], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[6], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return parseInt(results[7], 10);");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_date_time_long)
{
  // Creating the regexp for the validator with date first, then time
  std::string format = "dd-MM-yyyy HH:mm:ss.zzz";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "(\\d{2})-(\\d{2})-(\\d{4}) ([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])\\.([0-9]{3})");
  BOOST_TEST(dateTimeInfo.yearGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[4], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[5], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[6], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return parseInt(results[7], 10);");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_date_time_with_quote)
{
  // Creating the regexp for the validator with a literal in the format
  std::string format = "dd-MM-yyyy 'zzz' HH:mm:ss";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "(\\d{2})-(\\d{2})-(\\d{4}) zzz ([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])");
  BOOST_TEST(dateTimeInfo.yearGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[4], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[5], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[6], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return 0;");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_date_time_both)
{
  // Creating the regexp for the validator with date first, then time
  std::string format = "d-M-yy HH:m:ss.zzz AP";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "(\\d{1,2})-(\\d{1,2})-(\\d{2}) ([0-1][0-9]|2[0-3]):(0|[1-5]?[0-9]):([0-5][0-9])\\.([0-9]{3}) ([AP]M)");
  BOOST_TEST(dateTimeInfo.yearGetJS == "var y = parseInt(results[3], 10); return y > 38 ? 1900 + y : 2000 + y;");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[4], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[5], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[6], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return parseInt(results[7], 10);");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_interleaved)
{
  // Creating the regexp for the validator with messy formatting
  std::string format = "H dd yyyy --m--ss.zzz / MM";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "(0|[1-9]|1[0-9]|2[0-3]) (\\d{2}) (\\d{4}) --(0|[1-5]?[0-9])--([0-5][0-9])\\.([0-9]{3}) \\/ (\\d{2})");
  BOOST_TEST(dateTimeInfo.yearGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(dateTimeInfo.monthGetJS == "return parseInt(results[7], 10);");
  BOOST_TEST(dateTimeInfo.dayGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateTimeInfo.hourGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(dateTimeInfo.minuteGetJS == "return parseInt(results[4], 10);");
  BOOST_TEST(dateTimeInfo.secGetJS == "return parseInt(results[5], 10);");
  BOOST_TEST(dateTimeInfo.msecGetJS == "return parseInt(results[6], 10);");
}

bool hasMessage(const Wt::WException& exc, const std::string& msg)
{
  return std::string(exc.what()).find(msg) != std::string::npos;
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_year_not_allowed)
{
  // Creating the regexp for the validator with messy formatting
  std::string format = "HH mm ss AP dd MM yyy";
  BOOST_REQUIRE_EXCEPTION(Wt::WDateTime::formatToRegExp(format), Wt::WException, std::bind(&hasMessage, std::placeholders::_1, "3 consecutive y"));
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_month_allowed)
{
  // Adding too many month formats, result in exception.
  std::string format = "HH mm ss AP dd MMM yyyy";

  BOOST_REQUIRE_EXCEPTION(Wt::WDate::formatToRegExp(format), Wt::WException, std::bind(&hasMessage, std::placeholders::_1, "3 consecutive M"));
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_day_not_allowed)
{
  // Adding too many day formats, result in exception.
  std::string format = "HH mm ss AP ddddd MM yyyy";

  BOOST_REQUIRE_EXCEPTION(Wt::WDate::formatToRegExp(format), Wt::WException, std::bind(&hasMessage, std::placeholders::_1, "5 consecutive d"));
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_hour_allowed)
{
  // Adding too many hour formats, result in "duplication".
  std::string format = "HHHH mm ss AP dd MM yyyy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3])([0-1][0-9]|2[0-3]) ([0-5][0-9]) ([0-5][0-9]) ([AP]M) (\\d{2}) (\\d{2}) (\\d{4})");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_minute_allowed)
{
  // Adding too many minute formats, result in "duplication".
  std::string format = "HH mmm ss AP dd MM yyyy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]) ([0-5][0-9])(0|[1-5]?[0-9]) ([0-5][0-9]) ([AP]M) (\\d{2}) (\\d{2}) (\\d{4})");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_second_allowed)
{
  // Adding too many second formats, result in "duplication".
  std::string format = "HH mm sssss AP dd MM yyyy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]) ([0-5][0-9]) ([0-5][0-9])([0-5][0-9])(0|[1-5]?[0-9]) ([AP]M) (\\d{2}) (\\d{2}) (\\d{4})");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_millisecond_allowed)
{
  // Adding too many millisecond formats, result in duplication.
  std::string format = "HH mm ss.zzzz AP dd MM yyyy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]) ([0-5][0-9]) ([0-5][0-9])\\.([0-9]{3})(0|[1-9][0-9]{0,2}) ([AP]M) (\\d{2}) (\\d{2}) (\\d{4})");

  // Adding too many millisecond formats, result in an omission of 'zz'.
  format = "HH mm ss.zz AP dd MM yyyy";
  dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]) ([0-5][0-9]) ([0-5][0-9])\\. ([AP]M) (\\d{2}) (\\d{2}) (\\d{4})");

  format = "HH mm ss.zzzzz AP dd MM yyyy";
  dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]) ([0-5][0-9]) ([0-5][0-9])\\.([0-9]{3}) ([AP]M) (\\d{2}) (\\d{2}) (\\d{4})");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_bad_formatting_ap_allowed)
{
  // Adding too many ap formats (partial), result regular character match.
  std::string format = "HH mm ss APP dd MM yyyy";
  Wt::WDateTime::RegExpInfo dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]) ([0-5][0-9]) ([0-5][0-9]) ([AP]M)P (\\d{2}) (\\d{2}) (\\d{4})");

  format = "HH mm ss APA dd MM yyyy";
  dateTimeInfo = Wt::WDateTime::formatToRegExp(format);

  BOOST_TEST(dateTimeInfo.regexp == "([0-1][0-9]|2[0-3]) ([0-5][0-9]) ([0-5][0-9]) ([AP]M)([AP]M) (\\d{2}) (\\d{2}) (\\d{4})");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_standalone_meridiem)
{
  auto dateTimeInfo = Wt::WDateTime::formatToRegExp("h:mm A");

  BOOST_TEST(dateTimeInfo.regexp == "([1-9]|1[012]):([0-5][0-9]) ([AP]M)");

  dateTimeInfo = Wt::WDateTime::formatToRegExp("h:mm a");

  BOOST_TEST(dateTimeInfo.regexp == "([1-9]|1[012]):([0-5][0-9]) ([ap]m)");
}

BOOST_AUTO_TEST_CASE(WDateTime_formatToRegExp_quoted_ap_ignored_by_hours)
{
  auto dateTimeInfo = Wt::WDateTime::formatToRegExp("h:mm 'A'");

  BOOST_TEST(dateTimeInfo.regexp == "(0|[1-9]|1[0-9]|2[0-3]):([0-5][0-9]) A");

  dateTimeInfo = Wt::WDateTime::formatToRegExp("h:mm 'a'");

  BOOST_TEST(dateTimeInfo.regexp == "(0|[1-9]|1[0-9]|2[0-3]):([0-5][0-9]) a");

  dateTimeInfo = Wt::WDateTime::formatToRegExp("h:mm 'AP'");

  BOOST_TEST(dateTimeInfo.regexp == "(0|[1-9]|1[0-9]|2[0-3]):([0-5][0-9]) AP");

  dateTimeInfo = Wt::WDateTime::formatToRegExp("h:mm 'ap'");

  BOOST_TEST(dateTimeInfo.regexp == "(0|[1-9]|1[0-9]|2[0-3]):([0-5][0-9]) ap");
}
