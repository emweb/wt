// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

#include "WDateTimeTest.h"

#include <Wt/WDate>
#include <Wt/WTime>
#include <Wt/WDateTime>

void WDateTimeTest::test_WDate()
{
  Wt::WDate wd(2009, 10, 1);
  BOOST_REQUIRE(wd.toString() == "Thu Oct 1 2009");
}

void WDateTimeTest::test_WTime()
{
  Wt::WTime wt(12, 11, 31);
  BOOST_REQUIRE(wt.toString() == "12:11:31");
}

void WDateTimeTest::test_WDateTime()
{
  Wt::WDate wd(2009, 10, 1);
  Wt::WTime wt(12, 11, 31, 499);
  Wt::WDateTime wdt(wd, wt);

  BOOST_REQUIRE(wdt.toString() == "Thu Oct 1 12:11:31 2009");
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
		== "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt.time().msec() == 499);

  Wt::WDateTime wdt2 = wdt.addMSecs(1600);
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
		== "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt2.toString("ddd MMM d HH:mm:ss:zzz yyyy")
		== "Thu Oct 1 12:11:33:099 2009");
}

WDateTimeTest::WDateTimeTest()
  : test_suite("wdatetime_test_suite")
{
  add(BOOST_TEST_CASE(boost::bind(&WDateTimeTest::test_WDate, this)));
  add(BOOST_TEST_CASE(boost::bind(&WDateTimeTest::test_WTime, this)));
  add(BOOST_TEST_CASE(boost::bind(&WDateTimeTest::test_WDateTime, this)));
}
