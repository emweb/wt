/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WTime.h>

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_short)
{
  // Creating the regexp for the validator for short times
  std::string format = "H:m:s.z";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "(0|[1-9]|[1][0-9]|2[0-3]):(0|[1-5]?[0-9]):(0|[1-5]?[0-9])\\.(0|[1-9][0-9]{0,2})");
  BOOST_TEST(timeInfo.hourGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(timeInfo.minuteGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(timeInfo.secGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(timeInfo.msecGetJS == "return parseInt(results[4], 10);");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_long)
{
  // Creating the regexp for the validator for long times
  std::string format = "HH:mm:ss.zzz";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]):([0-5][0-9]):([0-5][0-9])\\.([0-9]{3})");
  BOOST_TEST(timeInfo.hourGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(timeInfo.minuteGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(timeInfo.secGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(timeInfo.msecGetJS == "return parseInt(results[4], 10);");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_both)
{
  // Creating the regexp for the validator for mixed times
  std::string format = "h:mm:s.zzz ap";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([1-9]|1[012]):([0-5][0-9]):(0|[1-5]?[0-9])\\.([0-9]{3}) ([ap]m)");
  BOOST_TEST(timeInfo.hourGetJS == "return parseInt(results[1], 10);");
  BOOST_TEST(timeInfo.minuteGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(timeInfo.secGetJS == "return parseInt(results[3], 10);");
  BOOST_TEST(timeInfo.msecGetJS == "return parseInt(results[4], 10);");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_bad_formatting_hour_allowed)
{
  // Creating the regexp for the validator with weird, but valid hour
  // format, which results in duplication.
  std::string format = "HHHH mm ss AP";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3])([0-1][0-9]|[2][0-3]) ([0-5][0-9]) ([0-5][0-9]) ([AP]M)");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_bad_formatting_minute_allowed)
{
  // Creating the regexp for the validator with weird, but valid minute
  // format, which results in duplication.
  std::string format = "HH mmm ss AP";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]) ([0-5][0-9])(0|[1-5]?[0-9]) ([0-5][0-9]) ([AP]M)");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_bad_formatting_second_allowed)
{
  // Creating the regexp for the validator with weird, but valid second
  // format, which results in duplication.
  std::string format = "HH mm sssss AP";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]) ([0-5][0-9]) ([0-5][0-9])([0-5][0-9])(0|[1-5]?[0-9]) ([AP]M)");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_bad_formatting_millisecond_allowed)
{
  // Creating the regexp for the validator with weird, but valid millisecond
  // format, which results in duplication.
  std::string format = "HH mm ss.zzzz AP";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]) ([0-5][0-9]) ([0-5][0-9])\\.([0-9]{3})(0|[1-9][0-9]{0,2}) ([AP]M)");

  format = "HH mm ss.zz AP";
  timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]) ([0-5][0-9]) ([0-5][0-9])\\. ([AP]M)");

  format = "HH mm ss.zzzzz AP";
  timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]) ([0-5][0-9]) ([0-5][0-9])\\.([0-9]{3}) ([AP]M)");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_bad_formatting_ap_allowed)
{
  // Creating the regexp for the validator with weird, but valid AP
  // format, which results in duplication.
  std::string format = "HH mm ss APP";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]) ([0-5][0-9]) ([0-5][0-9]) ([AP]M)P");
}

BOOST_AUTO_TEST_CASE(WTime_formatToRegExp_bad_formatting_ap_not_allowed)
{
  // Creating the regexp for the validator with weird, but valid AP
  // format, which results in duplication.
  std::string format = "HH mm ss APA";
  Wt::WTime::RegExpInfo timeInfo = Wt::WTime::formatToRegExp(format);

  BOOST_TEST(timeInfo.regexp == "([0-1][0-9]|[2][0-3]) ([0-5][0-9]) ([0-5][0-9]) ([AP]M)A");
}
