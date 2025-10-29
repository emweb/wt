/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WDate.h>
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

BOOST_AUTO_TEST_CASE( WDate_toString_non_localized )
{
  // Regression test for issue #11852
  // This produces a string that disregards the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDate date(2022, 1, 2);
  BOOST_TEST(date.toString("dd/MM/yyyy", false) == "02/01/2022");
  // January 2nd 2022 is a Sunday
  BOOST_TEST(date.toString("ddd MMM", false) == "Sun Jan");
  BOOST_TEST(date.toString("dddd MMMM", false) == "Sunday January");
}

BOOST_AUTO_TEST_CASE( WDate_toString_localized )
{
  // Regression test for issue #11852
  // This produces a string that takes the locale into account
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDate date(2022, 1, 2);
  BOOST_TEST(date.toString() == "Dom Ene 2 2022");
  BOOST_TEST(date.toString("dd/MM/yyyy") == "02/01/2022");
  // January 2nd 2022 is a Sunday
  BOOST_TEST(date.toString("ddd MMM") == "Dom Ene");
  BOOST_TEST(date.toString("dddd MMMM") == "Domigo Enero");
}

BOOST_AUTO_TEST_CASE( WDate_fromString_non_localized )
{
  // This parse a string disregarding the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDate date(2022, 1, 2);
  // Ensure date is valid
  BOOST_REQUIRE(date != Wt::WDate());

  BOOST_TEST((Wt::WDate::fromString("02/01/2022", "dd/MM/yyyy", false) == date));

  // Check that English names are recognized
  // January 2nd 2022 is a Sunday
  BOOST_TEST((Wt::WDate::fromString("Sun 02 Jan 2022", "ddd dd MMM yyyy", false) == date));
  BOOST_TEST((Wt::WDate::fromString("Sunday 02 January 2022", "dddd dd MMMM yyyy", false) == date));

  // Check that localized names are not recognized
  BOOST_TEST((Wt::WDate::fromString("Dom 02 Ene 2022", "ddd dd MMM yyyy", false) == Wt::WDate()));
  BOOST_TEST((Wt::WDate::fromString("Domigo 02 Enero 2022", "dddd dd MMMM yyyy", false) == Wt::WDate()));
}

BOOST_AUTO_TEST_CASE( WDate_fromString_localized )
{
  // This parse a string taking the locale into account
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  setup(app);

  Wt::WDate date(2022, 1, 2);
  // Ensure date is valid
  BOOST_REQUIRE(date != Wt::WDate());

  BOOST_TEST((Wt::WDate::fromString("02/01/2022", "dd/MM/yyyy") == date));

  // Check that localized names are recognized
  BOOST_TEST((Wt::WDate::fromString("Dom 02 Ene 2022", "ddd dd MMM yyyy") == date));
  BOOST_TEST((Wt::WDate::fromString("Domigo 02 Enero 2022", "dddd dd MMMM yyyy") == date));

  // Check that English names not recognized
  // January 2nd 2022 is a Sunday
  BOOST_TEST((Wt::WDate::fromString("Sun 02 Jan 2022", "ddd dd MMM yyyy") == Wt::WDate()));
  BOOST_TEST((Wt::WDate::fromString("Sunday 02 January 2022", "dddd dd MMMM yyyy") == Wt::WDate()));
}
