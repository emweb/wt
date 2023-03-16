/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/DateUtils.h"

#include <Wt/cpp20/date.hpp>

#include <chrono>

BOOST_AUTO_TEST_CASE( DateUtils_httpDate )
{
  using Wt::DateUtils::httpDate;
  using namespace std::chrono_literals;

  std::chrono::system_clock::time_point testDateTime{};

  // First two examples from: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
  testDateTime = static_cast<Wt::cpp20::date::sys_days>(Wt::cpp20::date::year(2015) / 10 / 21) + 7h + 28min + 0s;
  BOOST_TEST(httpDate(testDateTime) == "Wed, 21 Oct 2015 07:28:00 GMT");

  testDateTime = static_cast<Wt::cpp20::date::sys_days>(Wt::cpp20::date::year(2020) / 3 / 9) + 8h + 13min + 24s;
  BOOST_TEST(httpDate(testDateTime) == "Mon, 09 Mar 2020 08:13:24 GMT");

  // Example from: RFC9110, section 5.6.7
  testDateTime = static_cast<Wt::cpp20::date::sys_days>(Wt::cpp20::date::year(1994) / 11 / 6) + 8h + 49min + 37s;
  BOOST_TEST(httpDate(testDateTime) == "Sun, 06 Nov 1994 08:49:37 GMT");

  // Before UNIX epoch. Unlikely, but let's test it for good measure
  testDateTime = static_cast<Wt::cpp20::date::sys_days>(Wt::cpp20::date::year(1969) / 7 / 20) + 20h + 17min + 39s;
  BOOST_TEST(httpDate(testDateTime) == "Sun, 20 Jul 1969 20:17:39 GMT");
}
