/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WDate.h>
#include <Wt/WTime.h>
#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WStringStream.h>

#include <Wt/Date/tz.h>

#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <type_traits>

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
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDate3)
{
    Wt::WDate wd1(2009, 10, 2);
    Wt::WDate wd2(2010, 10, 2);
    BOOST_REQUIRE(wd1.daysTo(wd2) == 365);
    BOOST_REQUIRE(wd1.isLeapYear(2016));

    Wt::WDate wd3(2016, 2, 18); //Thursday
    BOOST_REQUIRE(wd3.dayOfWeek() == 4);
    BOOST_REQUIRE(wd3.toJulianDay() == 2457437);

    Wt::WDate wd4 = wd3.fromJulianDay(2457437);
    BOOST_REQUIRE(wd4.day() == 18);
    BOOST_REQUIRE(wd4.month() == 2);
    BOOST_REQUIRE(wd4.year() == 2016);

    /*Wt::WDate current = wd3.currentServerDate();
    BOOST_REQUIRE(current.day() == 1);
    BOOST_REQUIRE(current.month() == 3);
    BOOST_REQUIRE(current.year() == 2016);*/
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDate4 )
{
    Wt::WDate current(2016, 2, 24); //Wednesday
    Wt::WDate d = Wt::WDate::previousWeekday(current, 5);
    //Looking for previous Friday
    BOOST_REQUIRE(d.day() == 19);
    BOOST_REQUIRE(d.month() == 2);
    BOOST_REQUIRE(d.year() == 2016);
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDate5 )
{
  Wt::WDate wd = Wt::WDate::fromString("31/07/9999/not-valid", "dd/MM/yyyy");
  BOOST_REQUIRE(!wd.isValid());
  BOOST_REQUIRE(wd.isNull());
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDate6 )
{
  // Testing day being set to last valid day in resulting month/year when doing addMonths
  Wt::WDate wd(2019, 5, 31);
  wd = wd.addMonths(6);

  BOOST_REQUIRE(wd.isValid());
  BOOST_REQUIRE(wd.day() == 30);
  BOOST_REQUIRE(wd.month() == 11);
  BOOST_REQUIRE(wd.year() == 2019);
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDate7 )
{
  // Testing day being set to last valid day in resulting month/year when doing addYears
  Wt::WDate wd(2016, 2, 29);
  wd = wd.addYears(1);

  BOOST_REQUIRE(wd.isValid());
  BOOST_REQUIRE(wd.day() == 28);
  BOOST_REQUIRE(wd.month() == 2);
  BOOST_REQUIRE(wd.year() == 2017);
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WTime )
{
  Wt::WTime wt(22, 11, 31);
  BOOST_REQUIRE(wt.toString() == "22:11:31");

  std::chrono::duration<int, std::milli> duration = wt.toTimeDuration();
  BOOST_REQUIRE(duration == std::chrono::hours(22) + std::chrono::minutes(11) + std::chrono::seconds(31) + std::chrono::milliseconds(0));

  Wt::WTime wt2 = Wt::WTime::fromTimeDuration(duration);
  BOOST_REQUIRE(wt2.hour() == 22);
  BOOST_REQUIRE(wt2.minute() == 11);
  BOOST_REQUIRE(wt2.second() == 31);

  std::chrono::duration<int, std::milli> d = std::chrono::hours(13) +
          std::chrono::minutes(42) + std::chrono::seconds(8) + std::chrono::milliseconds(102);
  Wt::WTime wt3 = Wt::WTime::fromTimeDuration(d);
  BOOST_REQUIRE(wt3.hour() == 13);
  BOOST_REQUIRE(wt3.minute() == 42);
  BOOST_REQUIRE(wt3.second() == 8);
  BOOST_REQUIRE(wt3.msec() == 102);
  BOOST_REQUIRE(wt3.toTimeDuration() == d);

  Wt::WTime wt4(15, 53, 12, 100);
  BOOST_REQUIRE(wt4.toString() == "15:53:12");

  wt4 = wt4.addMSecs(900);
  BOOST_REQUIRE(wt4.toString() == "15:53:13");
  wt4 = wt4.addSecs(60);
  BOOST_REQUIRE(wt4.toString() == "15:54:13");

  BOOST_REQUIRE(wt4.secsTo(Wt::WTime(15, 54, 33)) == 20);

  Wt::WTime wt5 = Wt::WTime::currentServerTime();
  std::cerr << wt5.toString() << std::endl;

  Wt::WTime wt6 = Wt::WTime::fromString("11:42:16");
  BOOST_REQUIRE(wt6.hour() == 11);
  BOOST_REQUIRE(wt6.minute() == 42);
  BOOST_REQUIRE(wt6.second() == 16);
  wt6 = wt6.addMSecs(587);
  BOOST_REQUIRE(wt6.msec() == 587);
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WTime2 )
{
  Wt::WTime wt = Wt::WTime::fromString("13:05:12", "hh:mm:ss");
  BOOST_REQUIRE(wt.hour() == 13);
  BOOST_REQUIRE(wt.minute() == 05);
  BOOST_REQUIRE(wt.second() == 12);
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WTime3 )
{
  Wt::WTime wt = Wt::WTime::fromString("13:05:12:not-valid", "hh:mm:ss");
  BOOST_REQUIRE(!wt.isValid());
  BOOST_REQUIRE(wt.isNull());
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

  Wt::WDateTime wdt3 = wdt.addSecs(50);
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt3.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Thu Oct 1 12:12:21:499 2009");

  Wt::WDateTime wdt4 = wdt.addDays(1);
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt4.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Fri Oct 2 12:11:31:499 2009");

  Wt::WDateTime wdt5 = wdt.addMonths(1);
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt5.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Sun Nov 1 12:11:31:499 2009");

  Wt::WDateTime wdt6 = wdt.addYears(6);
  BOOST_REQUIRE(wdt.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Thu Oct 1 12:11:31:499 2009");
  BOOST_REQUIRE(wdt6.toString("ddd MMM d HH:mm:ss:zzz yyyy")
                == "Thu Oct 1 12:11:31:499 2015");

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

BOOST_AUTO_TEST_CASE( WDateTime_test_WDateTime2 )
{
  Wt::WDateTime wdt;

  BOOST_REQUIRE(!wdt.isValid());
  BOOST_REQUIRE(wdt.isNull());
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDateTime3 )
{
  Wt::WDateTime wdt = Wt::WDateTime::fromString("2000-06-14 13:05:12:invalid",
					        "yyyy-MM-dd hh:mm:ss");

  BOOST_REQUIRE(!wdt.isValid());
  /*
   * NOTE: This verifies that datetime is not null, matching current behavior
   * as checked in WDateTime_testspecial_WDateTime for construction from a
   * null date and null time....
   */
  BOOST_REQUIRE(!wdt.isNull());

  wdt = Wt::WDateTime::fromString("2000-06-14-invalid 13:05:12",
			          "yyyy-MM-dd hh:mm:ss");
  BOOST_REQUIRE(!wdt.isValid());
  BOOST_REQUIRE(!wdt.isNull());
}

BOOST_AUTO_TEST_CASE( WDateTime_test_WDateTime4 )
{
  Wt::WDateTime d
    = Wt::WDateTime::fromString("Sat, 24 Nov 2018 06:44:33 GMT",
				"ddd, dd MMM yyyy hh:mm:ss 'GMT'");

  BOOST_REQUIRE(d.date().year() == 2018);
  BOOST_REQUIRE(d.date().month() == 11);
  BOOST_REQUIRE(d.date().day() == 24);
  BOOST_REQUIRE(d.time().hour() == 6);
  BOOST_REQUIRE(d.time().minute() == 44);
  BOOST_REQUIRE(d.time().second() == 33);
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
  Wt::WLocalDateTime wldt = Wt::WLocalDateTime::offsetDateTime(wdt.toTimePoint(),
                                                               std::chrono::hours{-4},
                                                               loc.dateTimeFormat());

  BOOST_REQUIRE(wldt.toString() == "2009-10-01 08:11:31");
  BOOST_REQUIRE(wldt.timeZoneOffset() == -240); //4h behind

  Wt::WDateTime utc = wldt.toUTC();

  BOOST_REQUIRE(utc == wdt);
}

BOOST_AUTO_TEST_CASE( WDateTime_testspecial_WLocalDateTime )
{
  Wt::WDateTime wdt;

  Wt::WLocale loc;
  Wt::WLocalDateTime wldt = Wt::WLocalDateTime::offsetDateTime(std::chrono::system_clock::time_point{},
                                                                std::chrono::hours{-4},
                                                                loc.dateTimeFormat());
  wldt.setDateTime(Wt::WDate(1976, 6, 14), Wt::WTime(3, 0, 0), true);

  BOOST_REQUIRE(wldt.isValid());
  BOOST_REQUIRE(!wldt.isNull());

  Wt::WDateTime utc = wldt.toUTC();

  std::cerr << utc.toString() << std::endl;
}
