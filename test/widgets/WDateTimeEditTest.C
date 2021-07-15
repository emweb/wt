/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WCalendar.h"
#include "Wt/WDateTime.h"
#include "Wt/WLocale.h"
#include "Wt/WWebWidget.h"
#include <boost/test/unit_test.hpp>

#include <Wt/WDateTimeEdit.h>
#include <Wt/WDateTimeValidator.h>
#include <Wt/Test/WTestEnvironment.h>

#include <web/DomElement.h>

#include <memory>

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WCalendar_current_date )
{
  // Retrieving the WDateTimeEdit current date returns the calendar's
  // current date, which is in sync with the editor.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Populated with current date
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto dateTime = Wt::WDateTime::currentDateTime();

  BOOST_REQUIRE(dateEdit->currentYear() == dateTime.date().year());
  BOOST_REQUIRE(dateEdit->currentMonth() == dateTime.date().month());
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WCalendar_browseToNextYear )
{
  // Retrieving the WDateTimeEdit current date returns the calendar's
  // current date, which is in sync with the editor. This remains in sync
  // when browsing the calendar.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Populated with current date
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto dateTime = Wt::WDateTime::currentDateTime();
  dateEdit->browseToNextYear();

  BOOST_REQUIRE(dateEdit->currentYear() == dateTime.date().year() + 1);
  BOOST_REQUIRE(dateEdit->currentMonth() == dateTime.date().month());
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WCalendar_browseToNextMonth )
{
  // Retrieving the WDateTimeEdit current date returns the calendar's
  // current date, which is in sync with the editor. This remains in sync
  // when browsing the calendar.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Populated with current date
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto dateTime = Wt::WDateTime::currentDateTime().addMonths(1);
  dateEdit->browseToNextMonth();

  BOOST_REQUIRE(dateEdit->currentYear() == dateTime.date().year());
  BOOST_REQUIRE(dateEdit->currentMonth() == dateTime.date().month());
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WCalendar_browseToPreviousYear )
{
  // Retrieving the WDateTimeEdit current date returns the calendar's
  // current date, which is in sync with the editor. This remains in sync
  // when browsing the calendar.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Populated with current date
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto dateTime = Wt::WDateTime::currentDateTime();
  dateEdit->browseToPreviousYear();

  BOOST_REQUIRE(dateEdit->currentYear() == dateTime.date().year() - 1);
  BOOST_REQUIRE(dateEdit->currentMonth() == dateTime.date().month());
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WCalendar_browseToPreviousMonth )
{
  // Retrieving the WDateTimeEdit current date returns the calendar's
  // current date, which is in sync with the editor. This remains in sync
  // when browsing the calendar.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Populated with current date
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto dateTime = Wt::WDateTime::currentDateTime().addMonths(-1);
  dateEdit->browseToPreviousMonth();

  BOOST_REQUIRE(dateEdit->currentYear() == dateTime.date().year());
  BOOST_REQUIRE(dateEdit->currentMonth() == dateTime.date().month());
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WCalendar_browseTo )
{
  // Retrieving the WDateTimeEdit current date returns the calendar's
  // current date, which is in sync with the editor. This remains in sync
  // when browsing the calendar.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Populated with current date
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto dateTime = Wt::WDateTime::currentDateTime().addDays(128);
  dateEdit->browseTo(dateTime.date());

  BOOST_REQUIRE(dateEdit->currentYear() == dateTime.date().year());
  BOOST_REQUIRE(dateEdit->currentMonth() == dateTime.date().month());
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WCalendar_horizontalHeader )
{
  // Setting a value that changes the underlying WCalendar is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is ShortDayNames
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->horizontalHeaderFormat() == Wt::CalendarHeaderFormat::ShortDayNames);

  dateEdit->setHorizontalHeaderFormat(Wt::CalendarHeaderFormat::LongDayNames);

  BOOST_REQUIRE(dateEdit->horizontalHeaderFormat() == Wt::CalendarHeaderFormat::LongDayNames);
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WTimeEdit_hourStep )
{
  // Setting a value that changes the underlying WTimePicker is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is 1
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->hourStep() == 1);

  dateEdit->setHourStep(2);

  BOOST_REQUIRE(dateEdit->hourStep() == 2);
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WTimeEdit_minuteStep )
{
  // Setting a value that changes the underlying WTimePicker is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is 1
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->minuteStep() == 1);

  dateEdit->setMinuteStep(2);

  BOOST_REQUIRE(dateEdit->minuteStep() == 2);
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WTimeEdit_secondStep_disabled )
{
  // Setting a value that changes the underlying WTimePicker is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is 1, and seconds are enabled
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->secondStep() == 1);

  dateEdit->setFormat("yyyy-MM-dd hh:mm");
  dateEdit->setSecondStep(2);

  BOOST_REQUIRE(dateEdit->secondStep() == 1);
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WTimeEdit_secondStep_enabled )
{
  // Setting a value that changes the underlying WTimePicker is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is 1, and seconds are enabled
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->secondStep() == 1);

  dateEdit->setSecondStep(2);

  BOOST_REQUIRE(dateEdit->secondStep() == 2);
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WTimeEdit_millisecondStep_disabled )
{
  // Setting a value that changes the underlying WTimePicker is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is 1, and milliseconds are disabled
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->millisecondStep() == 1);

  dateEdit->setMillisecondStep(2);

  BOOST_REQUIRE(dateEdit->millisecondStep() == 1);
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WTimeEdit_millisecondStep_enabled )
{
  // Setting a value that changes the underlying WTimePicker is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is 1, and milliseconds are disabled
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->millisecondStep() == 1);

  dateEdit->setFormat("yyyy-MM-dd hh:mm:ss:zzz");
  dateEdit->setMillisecondStep(2);

  BOOST_REQUIRE(dateEdit->millisecondStep() == 2);
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_WTimeEdit_wrapAround )
{
  // Setting a value that changes the underlying WTimePicker is correctly
  // propagated.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  // Default is true
  auto dateEdit = std::make_unique<Wt::WDateTimeEdit>();

  BOOST_REQUIRE(dateEdit->wrapAroundEnabled());

  dateEdit->setWrapAroundEnabled(false);

  BOOST_REQUIRE(!dateEdit->wrapAroundEnabled());
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_setNativeControl_disable_with_validator )
{
  // Tests whether the non-native WDateTimeEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateTimeEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto currentDatetime = Wt::WDateTime::currentDateTime();
  dateTimeEdit->setValidator(std::make_shared<Wt::WDateTimeValidator>("yyyy-MM-dd hh:mm", currentDatetime, currentDatetime.addMonths(12)));

  // Simulate UI update
  dateTimeEdit->load();
  auto domElement = dateTimeEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateTimeEdit->styleClass() == "Wt-datetimeedit");
  BOOST_REQUIRE(dateTimeEdit->format() == "yyyy-MM-dd hh:mm");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_setNativeControl_disable_change_format )
{
  // Tests whether the non-native WDateTimeEdit has the right format.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateTimeEdit = std::make_unique<Wt::WDateTimeEdit>();

  // Simulate UI update
  dateTimeEdit->load();
  dateTimeEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateTimeEdit->format() == Wt::WLocale::currentLocale().dateTimeFormat());

  dateTimeEdit->setFormat("hh:mm dd-MM-yyyy");

  BOOST_REQUIRE(dateTimeEdit->format() == "hh:mm dd-MM-yyyy");
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_setNativeControl_enable_no_validator )
{
  // Tests whether the native WDateTimeEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateTimeEdit = std::make_unique<Wt::WDateTimeEdit>();
  dateTimeEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = dateTimeEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateTimeEdit->styleClass() == "");
  BOOST_REQUIRE(dateTimeEdit->format() == "yyyy-MM-dd'T'HH:mm");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_setNativeControl_enable_with_validator )
{
  // Tests whether the native WDateTimeEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateTimeEdit = std::make_unique<Wt::WDateTimeEdit>();
  auto currentDatetime = Wt::WDateTime::currentDateTime();
  dateTimeEdit->setValidator(std::make_shared<Wt::WDateTimeValidator>(currentDatetime, currentDatetime.addMonths(12)));
  dateTimeEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = dateTimeEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateTimeEdit->styleClass() == "");
  BOOST_REQUIRE(dateTimeEdit->format() == "yyyy-MM-dd'T'HH:mm");
  BOOST_REQUIRE(domElement->getAttribute("min") == currentDatetime.toString("yyyy-MM-dd'T'HH:mm"));
  BOOST_REQUIRE(domElement->getAttribute("max") == currentDatetime.addMonths(12).toString("yyyy-MM-dd'T'HH:mm"));

  delete domElement;
}

BOOST_AUTO_TEST_CASE( WDateTimeEdit_setNativeControl_enable_change_format )
{
  // Tests whether the native WDateTimeEdit has the right styleclass,
  // format and min/max attributes.
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto dateTimeEdit = std::make_unique<Wt::WDateTimeEdit>();
  dateTimeEdit->setNativeControl(true);

  // Simulate UI update
  auto domElement = dateTimeEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateTimeEdit->format() == "yyyy-MM-dd'T'HH:mm");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  delete domElement;

  dateTimeEdit->setFormat("hh:mm dd-MM-yyyy");

  domElement = dateTimeEdit->createSDomElement(&app);

  BOOST_REQUIRE(dateTimeEdit->format() == "yyyy-MM-dd'T'HH:mm");
  BOOST_REQUIRE(domElement->getAttribute("min") == "");
  BOOST_REQUIRE(domElement->getAttribute("max") == "");

  delete domElement;
}
