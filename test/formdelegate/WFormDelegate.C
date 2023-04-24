/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Form/WFormDelegate.h"

#include "Wt/WDate.h"
#include "Wt/WDateValidator.h"
#include "Wt/WFormModel.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLocale.h"
#include "Wt/WTime.h"
#include "Wt/WTimeValidator.h"
#include "Wt/WValidator.h"

#include "Wt/Test/WTestEnvironment.h"

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_updateModelValue_valid_date )
{
  // Testing that valid string input (correct date format) results in the model containing
  // a valid WDate object (i.e. string input gets converted to a WDate object).

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setDateFormat("yyyy-MM-dd");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("date-field");

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("2023-05-03");

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "date-field", edit.get());

  BOOST_TEST(formModel->valueText("date-field") == Wt::WDate(2023, 5, 3).toString("yyyy-MM-dd"));
  BOOST_CHECK(Wt::cpp17::any_cast<Wt::WDate>(formModel->value("date-field")) == Wt::WDate(2023, 5, 3));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_updateModelValue_invalid_date )
{
  // Testing that an invalid string input (incorrect date format) results in the model containing
  // the validation result instead of a WDate object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setDateFormat("yyyy-MM-dd");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("date-field");

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValidator(std::make_shared<Wt::WDateValidator>());
  edit->setValueText("03/05/2023");

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "date-field", edit.get());

  BOOST_CHECK(formModel->value("date-field").type() == typeid(Wt::WValidator::Result));
  BOOST_CHECK(Wt::cpp17::any_cast<Wt::WValidator::Result>(formModel->value("date-field")).state() == Wt::ValidationState::Invalid);
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_updateModelValue_widget_empty )
{
  // Testing that an empty string results in the model containing a default constructed
  // WDate object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setDateFormat("yyyy-MM-dd");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("date-field");

  auto edit = std::make_unique<Wt::WLineEdit>();

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "date-field", edit.get());

  BOOST_TEST(formModel->valueText("date-field").empty());
  BOOST_CHECK(Wt::cpp17::any_cast<Wt::WDate>(formModel->value("date-field")) == Wt::WDate());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_updateViewValue_model_contains_validation_result )
{
  // Testing that the view doesn't get updated when the model contains a validation result.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setDateFormat("yyyy-MM-dd");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("date-field");
  formModel->setValue("date-field", Wt::WValidator::Result());

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("03/05/2023"); // Invalid value is kept in the UI field

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "date-field", edit.get());

  BOOST_TEST(edit->valueText() == "03/05/2023");
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_updateViewValue_model_contains_date )
{
  // Testing that the view gets updated when the model contains a valid WDate object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setDateFormat("yyyy-MM-dd");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("date-field");
  formModel->setValue("date-field", Wt::WDate(2023, 5, 3));

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("30/05/2023");

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "date-field", edit.get());

  BOOST_TEST(edit->valueText() == "2023-05-03");
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_updateViewValue_model_contains_null_date )
{
  // Testing that the value in the view gets cleared when the model contains a null date,
  // i.e. a default constructed WDate object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setDateFormat("yyyy-MM-dd");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("date-field");
  formModel->setValue("date-field", Wt::WDate());

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("03/05/2023");

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "date-field", edit.get());

  BOOST_TEST(edit->valueText().empty());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_updateModelValue_valid_time )
{
  // Testing that a valid string input (correct time format) results in the model containing
  // a valid WTime object (i.e. string input gets converted to a WTime object).

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setTimeFormat("HH:mm:ss");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("time-field");

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("15:44:30");

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "time-field", edit.get());

  BOOST_TEST(formModel->valueText("time-field") == Wt::WTime(15, 44, 30).toString("HH:mm:ss"));
  BOOST_CHECK(Wt::cpp17::any_cast<Wt::WTime>(formModel->value("time-field")) == Wt::WTime(15, 44, 30));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_updateModelValue_invalid_time )
{
  // Testing that an invalid string input (incorrect time format) results in the model containing
  // the validation result instead of a WTime object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setTimeFormat("HH:mm:ss");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("time-field");

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValidator(std::make_shared<Wt::WTimeValidator>());
  edit->setValueText("150:44:30");

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "time-field", edit.get());

  BOOST_CHECK(formModel->value("time-field").type() == typeid(Wt::WValidator::Result));
  BOOST_CHECK(Wt::cpp17::any_cast<Wt::WValidator::Result>(formModel->value("time-field")).state() == Wt::ValidationState::Invalid);
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_updateModelValue_widget_empty )
{
  // Testing that an empty string results in the model containing a default constructed
  // WTime object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setTimeFormat("HH:mm:ss");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("time-field");

  auto edit = std::make_unique<Wt::WLineEdit>();

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "time-field", edit.get());

  BOOST_TEST(formModel->valueText("time-field").empty());
  BOOST_CHECK(Wt::cpp17::any_cast<Wt::WTime>(formModel->value("time-field")) == Wt::WTime());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_updateViewValue_model_contains_validation_result )
{
  // Testing that the view doesn't get updated when the model contains a validation result.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setTimeFormat("HH:mm:ss");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("time-field");
  formModel->setValue("time-field", Wt::WValidator::Result());

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("150:44:30"); // Invalid value is kept in the UI

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "time-field", edit.get());

  BOOST_TEST(edit->valueText() == "150:44:30");
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_updateViewValue_model_contains_time )
{
  // Testing that the view gets updated when the model contains a valid WTime object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setTimeFormat("HH:mm:ss");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("time-field");
  formModel->setValue("time-field", Wt::WTime(15, 44, 30));

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("293:55:10");

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "time-field", edit.get());

  BOOST_TEST(edit->valueText() == "15:44:30");
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_updateViewValue_model_contains_null_time )
{
  // Testing that the value in the view gets cleared when the model contains a null time,
  // i.e. a default constructed WTime object.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WLocale locale;
  locale.setTimeFormat("HH:mm:ss");
  app.setLocale(locale);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("time-field");
  formModel->setValue("time-field", Wt::WTime());

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("150:44:30");

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "time-field", edit.get());

  BOOST_TEST(edit->valueText().empty());
}
