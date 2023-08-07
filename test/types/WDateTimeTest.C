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

BOOST_AUTO_TEST_CASE( WDateTime_toString_non_localized )
{
  // Regression test for issue #11852
  // This produces a string that disregards the locale
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

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
  std::string file = app.appRoot() + "types/days_months_names";
  BOOST_REQUIRE(Wt::FileUtils::exists(file + ".xml"));

  app.messageResourceBundle().use(file);

  Wt::WDateTime datetime(Wt::WDate(2022, 1, 2), Wt::WTime(10, 8, 9));
  BOOST_TEST(datetime.toString() == "Dom Ene 2 10:08:09 2022");
  BOOST_TEST(datetime.toString("hh:mm:ss dd/MM/yyyy") == "10:08:09 02/01/2022");
  // January 2nd 2022 is a Sunday
  BOOST_TEST(datetime.toString("ddd MMM") == "Dom Ene");
  BOOST_TEST(datetime.toString("dddd MMMM") == "Domigo Enero");
}
