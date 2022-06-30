/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WCalendar.h>
#include <Wt/WDate.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDateValidator.h>
#include <Wt/Test/WTestEnvironment.h>

BOOST_AUTO_TEST_CASE( dateedit_validator_changed1 )
{
  // Regression test for issue #10578
  // Changing the bottom and top of the validator should change bottom and top of
  // date edit and calendar
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();
  auto dv = dateEdit->dateValidator();
  const auto bottom = Wt::WDate(1999, 1, 1);
  const auto top = Wt::WDate(2042, 1, 1);
  dv->setBottom(bottom);
  dv->setTop(top);

  BOOST_REQUIRE(dv->bottom() == bottom);
  BOOST_REQUIRE(dv->top() == top);
  BOOST_REQUIRE(dateEdit->bottom() == bottom);
  BOOST_REQUIRE(dateEdit->top() == top);

  auto calendar = dateEdit->calendar();
  BOOST_REQUIRE(calendar->bottom() == bottom);
  BOOST_REQUIRE(calendar->top() == top);
}

BOOST_AUTO_TEST_CASE( dateedit_validator_changed2 )
{
  // Regression test for issue #10578
  // Changing the bottom and top of the date edit should change bottom and top of
  // date validator and calendar
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();
  auto dv = dateEdit->dateValidator();
  const auto bottom = Wt::WDate(1999, 1, 1);
  const auto top = Wt::WDate(2042, 1, 1);
  dateEdit->setBottom(bottom);
  dateEdit->setTop(top);

  BOOST_REQUIRE(dv->bottom() == bottom);
  BOOST_REQUIRE(dv->top() == top);
  BOOST_REQUIRE(dateEdit->bottom() == bottom);
  BOOST_REQUIRE(dateEdit->top() == top);

  auto calendar = dateEdit->calendar();
  BOOST_REQUIRE(calendar->bottom() == bottom);
  BOOST_REQUIRE(calendar->top() == top);
}

BOOST_AUTO_TEST_CASE( dateedit_validator_changed3 )
{
  // Regression test for issue #10578
  // Changing the bottom and top of the date edit should change bottom and top of
  // the calendar, even if we have no validator
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();
  dateEdit->setValidator(nullptr);
  const auto bottom = Wt::WDate(1999, 1, 1);
  const auto top = Wt::WDate(2042, 1, 1);
  dateEdit->setBottom(bottom);
  dateEdit->setTop(top);

  BOOST_REQUIRE(dateEdit->bottom() == bottom);
  BOOST_REQUIRE(dateEdit->top() == top);

  auto calendar = dateEdit->calendar();
  BOOST_REQUIRE(calendar->bottom() == bottom);
  BOOST_REQUIRE(calendar->top() == top);
}
