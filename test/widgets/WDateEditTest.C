/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WWebWidget.h"
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WCalendar.h>
#include <Wt/WDate.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDateValidator.h>
#include <Wt/Test/WTestEnvironment.h>

#include <web/DomElement.h>

#include <memory>

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

BOOST_AUTO_TEST_CASE( WDateEdit_setNativeControl_disable_with_validator )
{
  // Tests whether the non-native WDateEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();
  dateEdit->setValidator(std::make_shared<Wt::WDateValidator>(Wt::WDate(2021, 1, 1), Wt::WDate(2022, 1, 1)));

  // Simulate UI update
  dateEdit->load();
  auto domElement = dateEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateEdit->styleClass() == "Wt-dateedit");
  BOOST_REQUIRE(dateEdit->format() == "yyyy-MM-dd");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}

BOOST_AUTO_TEST_CASE( WDateEdit_setNativeControl_disable_change_format )
{
  // Tests whether the non-native WDateEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();

  // Simulate UI update
  dateEdit->load();
  auto domElement = dateEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateEdit->format() == "yyyy-MM-dd");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  dateEdit->setFormat("dd-MM-yyyy");

  BOOST_REQUIRE(dateEdit->format() == "dd-MM-yyyy");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}

BOOST_AUTO_TEST_CASE( WDateEdit_setNativeControl_enable_no_validator )
{
  // Tests whether the native WDateEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();
  dateEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = dateEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateEdit->styleClass() == "");
  BOOST_REQUIRE(dateEdit->format() == "yyyy-MM-dd");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}

BOOST_AUTO_TEST_CASE( WDateEdit_setNativeControl_enable_with_validator )
{
  // Tests whether the native WDateEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();
  dateEdit->setValidator(std::make_shared<Wt::WDateValidator>(Wt::WDate(2021, 1, 1), Wt::WDate(2022, 1, 1)));
  dateEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = dateEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateEdit->styleClass() == "");
  BOOST_REQUIRE(dateEdit->format() == "yyyy-MM-dd");
  BOOST_REQUIRE(domElement->getAttribute("min") == "2021-01-01");
  BOOST_REQUIRE(domElement->getAttribute("max") == "2022-01-01");
}

BOOST_AUTO_TEST_CASE( WDateEdit_setNativeControl_enable_change_format )
{
  // Tests whether the native WDateEdit always has the same format
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateEdit = std::make_unique<Wt::WDateEdit>();
  dateEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = dateEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateEdit->format() == "yyyy-MM-dd");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  dateEdit->setFormat("dd-MM-yyyy");

  BOOST_REQUIRE(dateEdit->format() == "yyyy-MM-dd");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}
