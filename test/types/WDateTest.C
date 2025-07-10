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

BOOST_AUTO_TEST_CASE(WDate_formatToRegExp_short)
{
  // Creating the regexp for the validator for short dates
  std::string format = "d-M-yy";
  Wt::WDate::RegExpInfo dateInfo = Wt::WDate::formatToRegExp(format);

  BOOST_TEST(dateInfo.regexp == "(\\d{1,2})-(\\d{1,2})-(\\d{2})");
  BOOST_TEST(dateInfo.yearGetJS == "var y=parseInt(results[3], 10);return y > 38 ? 1900 + y : 2000 + y;");
  BOOST_TEST(dateInfo.monthGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateInfo.dayGetJS == "return parseInt(results[1], 10);");
}

BOOST_AUTO_TEST_CASE(WDate_formatToRegExp_long)
{
  // Creating the regexp for the validator for long dates
  std::string format = "dd-MM-yyyy";
  Wt::WDate::RegExpInfo dateInfo = Wt::WDate::formatToRegExp(format);

  BOOST_TEST(dateInfo.regexp == "(\\d{2})-(\\d{2})-(\\d{4})");
  BOOST_TEST(dateInfo.yearGetJS == "return parseInt(results[3], 10)");
  BOOST_TEST(dateInfo.monthGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateInfo.dayGetJS == "return parseInt(results[1], 10);");
}

BOOST_AUTO_TEST_CASE(WDate_formatToRegExp_both)
{
  // Creating the regexp for the validator for mixed dates
  std::string format = "dd-M-yy";
  Wt::WDate::RegExpInfo dateInfo = Wt::WDate::formatToRegExp(format);

  BOOST_TEST(dateInfo.regexp == "(\\d{2})-(\\d{1,2})-(\\d{2})");
  BOOST_TEST(dateInfo.yearGetJS == "var y=parseInt(results[3], 10);return y > 38 ? 1900 + y : 2000 + y;");
  BOOST_TEST(dateInfo.monthGetJS == "return parseInt(results[2], 10);");
  BOOST_TEST(dateInfo.dayGetJS == "return parseInt(results[1], 10);");
}

namespace {
  bool hasMessage(const Wt::WException& exc, const std::string& msg)
  {
    std::cout << exc.what() << std::endl;
    return std::string(exc.what()).find(msg) != std::string::npos;
  }
}

BOOST_AUTO_TEST_CASE(WDate_formatToRegExp_bad_formatting_year_not_allowed)
{
  // Creating the regexp for the validator with invalid year format
  std::string format = "dd MM yyy";

  BOOST_REQUIRE_EXCEPTION(Wt::WDate::formatToRegExp(format), Wt::WException, std::bind(&hasMessage, std::placeholders::_1, "3 consecutive y"));

  format = "dd MM yyyyy";

  BOOST_REQUIRE_EXCEPTION(Wt::WDate::formatToRegExp(format), Wt::WException, std::bind(&hasMessage, std::placeholders::_1, "5 consecutive y"));
}

BOOST_AUTO_TEST_CASE(WDate_formatToRegExp_bad_formatting_month_allowed)
{
  // Creating the regexp for the validator with invalid month format
  std::string format = "dd MMM yyyy";

  BOOST_REQUIRE_EXCEPTION(Wt::WDate::formatToRegExp(format), Wt::WException, std::bind(&hasMessage, std::placeholders::_1, "3 consecutive M"));
}

BOOST_AUTO_TEST_CASE(WDate_formatToRegExp_bad_formatting_day_allowed)
{
  // Creating the regexp for the validator with invalid day format
  std::string format = "ddddd MM yyyy";

  BOOST_REQUIRE_EXCEPTION(Wt::WDate::formatToRegExp(format), Wt::WException, std::bind(&hasMessage, std::placeholders::_1, "5 consecutive d"));
}
