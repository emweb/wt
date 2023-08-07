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

BOOST_AUTO_TEST_CASE( WDate_toString_non_localized )
{
  // Regression test for issue #11852
  // This produces a string that disregards the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WDate date(2022, 1, 2);
  BOOST_TEST(date.toString("dd/MM/yyyy", false) == "02/01/2022");
  // January 2nd 2022 is a Sunday
  BOOST_TEST(date.toString("ddd MMM", false) == "Sun Jan");
  BOOST_TEST(date.toString("dddd MMMM", false) == "Sunday January");
}

BOOST_AUTO_TEST_CASE( WDate_toString_localized )
{
  // Regression test for issue #11852
  // This produces a string that disregards the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  std::string file = app.appRoot() + "types/days_months_names";
  BOOST_REQUIRE(Wt::FileUtils::exists(file + ".xml"));

  app.messageResourceBundle().use(file);

  Wt::WDate date(2022, 1, 2);
  BOOST_TEST(date.toString() == "Dom Ene 2 2022");
  BOOST_TEST(date.toString("dd/MM/yyyy") == "02/01/2022");
  // January 2nd 2022 is a Sunday
  BOOST_TEST(date.toString("ddd MMM") == "Dom Ene");
  BOOST_TEST(date.toString("dddd MMMM") == "Domigo Enero");
}
