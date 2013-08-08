/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WDate>
#include <Wt/WTime>
#include <Wt/WDateTime>
#include <Wt/WLocalDateTime>

BOOST_AUTO_TEST_CASE( WDateTime_test_WDate )
{
  Wt::WDate wd(2009, 10, 1);
  BOOST_REQUIRE(wd.toString() == "Thu Oct 1 2009");
  BOOST_REQUIRE(wd.isValid());
  BOOST_REQUIRE(!wd.isNull());
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDate2 )
{
  Wt::WDate wd;
  BOOST_REQUIRE(!wd.isValid());
  BOOST_REQUIRE(wd.isNull());

  wd.setDate(40, 50, 2000);

  BOOST_REQUIRE(!wd.isValid());
  BOOST_REQUIRE(!wd.isNull());

  wd = Wt::WDate::fromString("31/07/9999", "dd/MM/yyyy");
  
  BOOST_REQUIRE(wd.isValid());
  BOOST_REQUIRE(!wd.isNull());

  wd = wd.addDays(360);

  BOOST_REQUIRE(!wd.isValid());
  BOOST_REQUIRE(wd.isNull());
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WTime )
{
  Wt::WTime wt(12, 11, 31);
  BOOST_REQUIRE(wt.toString() == "12:11:31");
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDateTime )
{
  Wt::WDate wd(2009, 10, 1);
  Wt::WTime wt(12, 11, 31, 499);
  Wt::WDateTime wdt(wd, wt);

  BOOST_REQUIRE(wdt.isValid());
  BOOST_REQUIRE(!wdt.isNull());

  BOOST_REQUIRE(wdt.toString() == "Thu Oct 1 12:11:31 2009");
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
		== "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt.time().msec() == 499);

  Wt::WDateTime wdt2 = wdt.addMSecs(1600);
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
		== "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt2.toString("ddd MMM d HH:mm:ss:zzz yyyy")
		== "Thu Oct 1 12:11:33:099 2009");

  Wt::WDateTime d = Wt::WDateTime::fromString("2000-06-14 13:05:12",
					      "yyyy-MM-dd hh:mm:ss");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 13);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  d = Wt::WDateTime::fromString("2000-06-14 13:05:12",
				"yyyy-MM-dd HH:mm:ss");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 13);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  d = Wt::WDateTime::fromString("2000-06-14 1:05:12 AM",
				"yyyy-MM-dd h:mm:ss AP");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 1);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  d = Wt::WDateTime::fromString("2000-06-14 1:05:12 pm",
				"yyyy-MM-dd h:mm:ss ap");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 13);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  d = Wt::WDateTime::fromString("2000-06-14 1:05:12 PM",
				"yyyy-MM-dd h:mm:ss a");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 13);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  d = Wt::WDateTime::fromString("2000-06-14 1:05:12 AM",
				"yyyy-MM-dd h:mm:ss a");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 1);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  d = Wt::WDateTime::fromString("2000-06-14 1:05:12",
				"yyyy-MM-dd h:mm:ss");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 1);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  d = Wt::WDateTime::fromString("2000-06-14 13:05:12",
				"yyyy-MM-dd h:mm:ss");

  BOOST_REQUIRE(d.date().year() == 2000);
  BOOST_REQUIRE(d.date().month() == 6);
  BOOST_REQUIRE(d.date().day() == 14);
  BOOST_REQUIRE(d.time().hour() == 13);
  BOOST_REQUIRE(d.time().minute() == 05);
  BOOST_REQUIRE(d.time().second() == 12);

  BOOST_REQUIRE(Wt::WDateTime::fromString(d.toString()) == d);

  BOOST_REQUIRE(d.toString("ddd, MMM dd, yyyy; hh:mm:ss ap")
		== "Wed, Jun 14, 2000; 01:05:12 pm");
  BOOST_REQUIRE(d.toString("ddd, MMM dd, yyyy; hh:mm:ss 'a'")
		== "Wed, Jun 14, 2000; 13:05:12 a");
  BOOST_REQUIRE(d.toString("ddd, MMM dd, yyyy; hh:mm:ss")
		== "Wed, Jun 14, 2000; 13:05:12");
}

BOOST_AUTO_TEST_CASE( WDateTime_testspecial_WDateTime )
{
  Wt::WDateTime wdt;

  BOOST_REQUIRE(!wdt.isValid());
  BOOST_REQUIRE(wdt.isNull());

  wdt = Wt::WDateTime(Wt::WDate(), Wt::WTime());

  BOOST_REQUIRE(!wdt.isValid());
  BOOST_REQUIRE(!wdt.isNull());

  wdt = Wt::WDateTime(Wt::WDate(20, 30, 40), Wt::WTime());

  BOOST_REQUIRE(!wdt.isValid());
  BOOST_REQUIRE(!wdt.isNull());
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WLocalDateTime )
{
  Wt::WDate wd(2009, 10, 1);
  Wt::WTime wt(12, 11, 31, 499);
  Wt::WDateTime wdt(wd, wt);

  Wt::WLocale loc;
  loc.setTimeZone("EST-5EDT,M4.1.0,M10.5.0");

  Wt::WLocalDateTime wldt = wdt.toLocalTime(loc);

  BOOST_REQUIRE(wldt.toString() == "2009-10-01 08:11:31");

  Wt::WDateTime utc = wldt.toUTC();

  BOOST_REQUIRE(utc == wdt);
}

BOOST_AUTO_TEST_CASE( WDateTime_testspecial_WLocalDateTime )
{
  Wt::WDateTime wdt;

  Wt::WLocale loc;
  loc.setTimeZone("EST-5EDT,M4.1.0,M10.5.0");

  Wt::WLocalDateTime wldt = wdt.toLocalTime(loc);

  BOOST_REQUIRE(!wldt.isValid());
  BOOST_REQUIRE(wldt.isNull());

  Wt::WDateTime utc = wldt.toUTC();

  BOOST_REQUIRE(utc == wdt);

  wldt.setDateTime(Wt::WDate(1976,6,14), Wt::WTime(3,0,0));

  BOOST_REQUIRE(wldt.isValid());
  BOOST_REQUIRE(!wldt.isNull());

  utc = wldt.toUTC();

  std::cerr << utc.toString() << std::endl;
}
