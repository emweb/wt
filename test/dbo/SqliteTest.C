/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/cpp20/date.hpp>

#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include <Wt/Dbo/backend/Sqlite3.h>

#include <chrono>
#include <tuple>

namespace dbo = Wt::Dbo;
namespace dt = Wt::cpp20::date;

namespace {
  class A final : public dbo::Dbo<A> {
  public:
    template<typename Action>
    void persist(Action& a)
    {
      dbo::field(a, date, "date");
      dbo::field(a, dateTime, "date_time");
      dbo::field(a, timePoint, "time_point");
    }

    Wt::WDate date{};
    Wt::WDateTime dateTime{};
    std::chrono::system_clock::time_point timePoint{};
  };
}

BOOST_AUTO_TEST_CASE( sqlite3_test_iso_timestamp )
{
  auto sqlite3 = std::make_unique<dbo::backend::Sqlite3>(":memory:");
  sqlite3->setProperty("show-queries", "true");
  sqlite3->setDateTimeStorage(dbo::SqlDateTimeType::Date, dbo::backend::DateTimeStorage::ISO8601AsText);
  sqlite3->setDateTimeStorage(dbo::SqlDateTimeType::DateTime, dbo::backend::DateTimeStorage::ISO8601AsText);
  dbo::Session session;
  session.setConnection(std::move(sqlite3));
  session.mapClass<A>("a");
  session.createTables();

  Wt::WDate date;
  Wt::WDateTime dateTime;
  std::chrono::system_clock::time_point timePoint{};
  {
    using namespace std::chrono_literals;
    date = Wt::WDate(1950, 3, 12);
    dateTime = Wt::WDateTime(Wt::WDate(1969, 12, 31), Wt::WTime(23, 59, 59, 999));
    timePoint = static_cast<dt::sys_days>(dt::year(1990) / 2 / 14) + 1h + 50min + 5s + 12ms;
  }

  {
    dbo::Transaction t(session);
    auto a = session.addNew<A>();
    a.modify()->date = date;
    a.modify()->dateTime = dateTime;
    a.modify()->timePoint = timePoint;
  }

  {
    dbo::Transaction t(session);
    std::string dateStr, dateTimeStr, timePointStr;
    std::tie(dateStr, dateTimeStr, timePointStr) =
      session.query<std::tuple<std::string, std::string, std::string>>(
        "SELECT date, date_time, time_point FROM a").limit(1).resultValue();
    BOOST_CHECK_EQUAL(dateStr, "1950-03-12");
    BOOST_CHECK_EQUAL(dateTimeStr, "1969-12-31T23:59:59.999");
    BOOST_CHECK_EQUAL(timePointStr, "1990-02-14T01:50:05.012");
  }

  {
    dbo::Transaction t(session);
    auto a = session.find<A>().limit(1).resultValue();
    BOOST_CHECK(a->date == date);
    BOOST_CHECK(a->dateTime == dateTime);
    BOOST_CHECK(a->timePoint == timePoint);
  }
}

BOOST_AUTO_TEST_CASE( sqlite3_test_julian_day )
{
  auto sqlite3 = std::make_unique<dbo::backend::Sqlite3>(":memory:");
  sqlite3->setProperty("show-queries", "true");
  sqlite3->setDateTimeStorage(dbo::SqlDateTimeType::Date, dbo::backend::DateTimeStorage::JulianDaysAsReal);
  sqlite3->setDateTimeStorage(dbo::SqlDateTimeType::DateTime, dbo::backend::DateTimeStorage::JulianDaysAsReal);
  dbo::Session session;
  session.setConnection(std::move(sqlite3));
  session.mapClass<A>("a");
  session.createTables();

  Wt::WDate date;
  Wt::WDateTime dateTime;
  std::chrono::system_clock::time_point timePoint{};
  {
    using namespace std::chrono_literals;
    date = Wt::WDate(1950, 3, 12);
    dateTime = Wt::WDateTime(Wt::WDate(1948, 10, 2), Wt::WTime(3, 5, 12, 9));
    timePoint = static_cast<dt::sys_days>(dt::year(1990) / 2 / 14) + 1h + 50min + 5s + 12ms;
  }

  {
    dbo::Transaction t(session);
    auto a = session.addNew<A>();
    a.modify()->date = date;
    a.modify()->dateTime = dateTime;
    a.modify()->timePoint = timePoint;
  }

  {
    dbo::Transaction t(session);
    std::string dateStr, dateTimeStr, timePointStr;
    std::tie(dateStr, dateTimeStr, timePointStr) =
      session.query<std::tuple<std::string, std::string, std::string>>(
        R"=(
          SELECT
            strftime('%Y-%m-%d', date),
            strftime('%Y-%m-%dT%H:%M:%f', date_time),
            strftime('%Y-%m-%dT%H:%M:%f', time_point)
          FROM a
        )=").limit(1).resultValue();
    BOOST_CHECK_EQUAL(dateStr, "1950-03-12");
    BOOST_CHECK_EQUAL(dateTimeStr, "1948-10-02T03:05:12.009");
    BOOST_CHECK_EQUAL(timePointStr, "1990-02-14T01:50:05.012");

    double dateDbl1{}, dateDbl2{}, dateTimeDbl1{}, dateTimeDbl2{}, timePointDbl1{}, timePointDbl2{};
    std::tie(dateDbl1, dateDbl2, dateTimeDbl1, dateTimeDbl2, timePointDbl1, timePointDbl2) =
      session.query<std::tuple<double, double, double, double, double, double>>(
        R"=(
          SELECT
            date,
            julianday('1950-03-12'),
            date_time,
            julianday('1948-10-02T03:05:12.009'),
            time_point,
            julianday('1990-02-14T01:50:05.012')
          FROM a
        )=").limit(1).resultValue();
    BOOST_CHECK_EQUAL(dateDbl1, dateDbl2);
    BOOST_CHECK_EQUAL(dateTimeDbl1, dateTimeDbl2);
    BOOST_CHECK_EQUAL(timePointDbl1, timePointDbl2);
  }

  {
    using namespace std::chrono_literals;
    dbo::Transaction t(session);
    auto a = session.find<A>().limit(1).resultValue();
    BOOST_CHECK(a->date == date);
    BOOST_CHECK(dt::round<std::chrono::milliseconds>(a->dateTime.toTimePoint()) == dateTime.toTimePoint());
    BOOST_CHECK_EQUAL(a->dateTime.toString("yyyy-MM-ddTHH:mm:ss.zzz").toUTF8(),
                      dateTime.toString("yyyy-MM-ddTHH:mm:ss.zzz").toUTF8());
    BOOST_CHECK(dt::round<std::chrono::milliseconds>(a->timePoint) == timePoint);
  }
}

BOOST_AUTO_TEST_CASE( sqlite3_test_unix_timestamp )
{
  auto sqlite3 = std::make_unique<dbo::backend::Sqlite3>(":memory:");
  sqlite3->setProperty("show-queries", "true");
  sqlite3->setDateTimeStorage(dbo::SqlDateTimeType::Date, dbo::backend::DateTimeStorage::UnixTimeAsInteger);
  sqlite3->setDateTimeStorage(dbo::SqlDateTimeType::DateTime, dbo::backend::DateTimeStorage::UnixTimeAsInteger);
  dbo::Session session;
  session.setConnection(std::move(sqlite3));
  session.mapClass<A>("a");
  session.createTables();

  Wt::WDate date;
  Wt::WDateTime dateTime;
  std::chrono::system_clock::time_point timePoint{};
  {
    using namespace std::chrono_literals;
    date = Wt::WDate(1950, 3, 12);
    dateTime = Wt::WDateTime(Wt::WDate(1948, 10, 2), Wt::WTime(3, 5, 12));
    timePoint = static_cast<dt::sys_days>(dt::year(1990) / 2 / 14) + 1h + 50min + 5s;
  }

  {
    dbo::Transaction t(session);
    auto a = session.addNew<A>();
    a.modify()->date = date;
    a.modify()->dateTime = dateTime;
    a.modify()->timePoint = timePoint;
  }

  {
    dbo::Transaction t(session);
    long long dateLL1{}, dateLL2{}, dateTimeLL1{}, dateTimeLL2{}, timePointLL1{}, timePointLL2{};
    std::tie(dateLL1, dateLL2, dateTimeLL1, dateTimeLL2, timePointLL1, timePointLL2) =
      session.query<std::tuple<long long, long long, long long, long long, long long, long long>>(
        R"=(
          SELECT
            date,
            strftime('%s', '1950-03-12'),
            date_time,
            strftime('%s', '1948-10-02T03:05:12'),
            time_point,
            strftime('%s', '1990-02-14T01:50:05')
          FROM a
        )=").limit(1).resultValue();
    BOOST_CHECK_EQUAL(dateLL1, dateLL2);
    BOOST_CHECK_EQUAL(dateTimeLL1, dateTimeLL2);
    BOOST_CHECK_EQUAL(timePointLL1, timePointLL2);
  }

  {
    dbo::Transaction t(session);
    auto a = session.find<A>().limit(1).resultValue();
    BOOST_CHECK(a->date == date);
    BOOST_CHECK(a->dateTime == dateTime);
    BOOST_CHECK(a->timePoint == timePoint);
  }
}
