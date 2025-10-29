/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
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
