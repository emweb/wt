/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WTime.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WTimeValidator.h>

#include <Wt/Test/WTestEnvironment.h>

#include <chrono>
#include <web/DomElement.h>

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_CASE( WTimeEdit_setNativeControl_disable_with_validator )
{
  // Tests whether the non-native WTimeEdit has the right styleclass,
  // format, step, and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto timeEdit = std::make_unique<Wt::WTimeEdit>();
  auto validator = std::make_shared<Wt::WTimeValidator>("HH:mm:ss", Wt::WTime(10, 0, 0), Wt::WTime(11, 0, 0));
  timeEdit->setValidator(validator);

  // Simulate UI update
  timeEdit->load();
  auto domElement = timeEdit->createSDomElement(&app);

  BOOST_REQUIRE(timeEdit->styleClass() == "Wt-timeedit");
  BOOST_REQUIRE(timeEdit->format() == "HH:mm:ss");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(0));
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}

BOOST_AUTO_TEST_CASE( WTimeEdit_setNativeControl_disable_change_format )
{
  // Tests whether the non-native WTimeEdit correctly changes its format
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto timeEdit = std::make_unique<Wt::WTimeEdit>();

  // Simulate UI update
  timeEdit->load();
  auto domElement = timeEdit->createSDomElement(&app);

  auto validator = dynamic_cast<Wt::WTimeValidator*>(timeEdit->validator().get());
  BOOST_REQUIRE(timeEdit->format() == "HH:mm:ss");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(0));
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  timeEdit->setFormat("HH:mm");

  BOOST_REQUIRE(timeEdit->format() == "HH:mm");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(0));
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}

BOOST_AUTO_TEST_CASE( WTimeEdit_setNativeControl_enable_no_validator )
{
  // Tests whether the native WTimeEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto timeEdit = std::make_unique<Wt::WTimeEdit>();
  timeEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = timeEdit->createSDomElement(&app);

  auto validator = dynamic_cast<Wt::WTimeValidator*>(timeEdit->validator().get());
  BOOST_REQUIRE(timeEdit->styleClass() == "");
  BOOST_REQUIRE(timeEdit->format() == "HH:mm:ss");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(1));
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}

BOOST_AUTO_TEST_CASE( WTimeEdit_setNativeControl_enable_with_validator )
{
  // Tests whether the native WTimeEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto timeEdit = std::make_unique<Wt::WTimeEdit>();
  auto validator = std::make_shared<Wt::WTimeValidator>("HH:mm:ss", Wt::WTime(10, 0, 0), Wt::WTime(11, 0, 0));
  timeEdit->setValidator(validator);
  timeEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = timeEdit->createSDomElement(&app);

  BOOST_REQUIRE(timeEdit->styleClass() == "");
  BOOST_REQUIRE(timeEdit->format() == "HH:mm:ss");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(1));
  BOOST_REQUIRE(domElement->getAttribute("min") == "10:00:00");
  BOOST_REQUIRE(domElement->getAttribute("max") == "11:00:00");
}

BOOST_AUTO_TEST_CASE( WTimeEdit_setNativeControl_enable_change_format )
{
  // Tests whether the native WTimeEdit always has the same format
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto timeEdit = std::make_unique<Wt::WTimeEdit>();
  timeEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = timeEdit->createSDomElement(&app);

  auto validator = dynamic_cast<Wt::WTimeValidator*>(timeEdit->validator().get());
  BOOST_REQUIRE(timeEdit->format() == "HH:mm:ss");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(1));
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  timeEdit->setFormat("HH:mm");

  BOOST_REQUIRE(timeEdit->format() == "HH:mm");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(60));
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}

BOOST_AUTO_TEST_CASE( WTimeEdit_setNativeControl_enable_change_format_validator )
{
  // Tests whether the native WTimeEdit will misbehave if the format of
  // the validator is changed instead of on the widget itself.
  // This does change the format of the widget, but not its "settings",
  // like step.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto timeEdit = std::make_unique<Wt::WTimeEdit>();
  timeEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = timeEdit->createSDomElement(&app);

  auto validator = dynamic_cast<Wt::WTimeValidator*>(timeEdit->validator().get());
  BOOST_REQUIRE(timeEdit->format() == "HH:mm:ss");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(1));
  BOOST_REQUIRE(domElement->getAttribute("step") == "1");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  // Use the validator to set the format, which is wrong.
  validator->setFormat("HH:mm");

  BOOST_REQUIRE(timeEdit->format() == "HH:mm");
  BOOST_REQUIRE(validator->step() == std::chrono::seconds(60));
  // Wrong step for the widget
  BOOST_REQUIRE(domElement->getAttribute("step") == "1");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");
}
