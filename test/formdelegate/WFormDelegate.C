/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Form/WFormDelegate.h"

#include "Wt/WCheckBox.h"
#include "Wt/WDate.h"
#include "Wt/WDateEdit.h"
#include "Wt/WDateValidator.h"
#include "Wt/WFormModel.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLocale.h"
#include "Wt/WTime.h"
#include "Wt/WTimeEdit.h"
#include "Wt/WTimeValidator.h"
#include "Wt/WValidator.h"

#include "Wt/Test/WTestEnvironment.h"

BOOST_AUTO_TEST_CASE( WFormDelegate_WString_createFormWidget )
{
  // Testing that createFormWidget returns the expected widget.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<Wt::WString> formDelegate;
  auto widget = formDelegate.createFormWidget();

  BOOST_TEST(dynamic_cast<Wt::WLineEdit*>(widget.get()));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WString_createValidator)
{
  // Testing that there is no default validator for WString objects.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<Wt::WString> formDelegate;
  BOOST_TEST(!formDelegate.createValidator());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WString_updateModelValue )
{
  // Testing that the value in the model gets correctly updated
  // and that the value in view remains unchanged.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("wstring-field");
  formModel->setValue("wstring-field", "Hello");

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("Hi");

  Wt::Form::WFormDelegate<Wt::WString> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "wstring-field", edit.get());

  BOOST_TEST(formModel->valueText("wstring-field") == "Hi");
  BOOST_CHECK(formModel->value("wstring-field").type() == typeid(Wt::WString));

  BOOST_TEST(edit->valueText() == "Hi");
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WString_updateModelValue_empty )
{
  // Testing that an empty value in the view will result in clearing the value in the model
  // and that the value in the view remains unchanged.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("wstring-field");
  formModel->setValue("wstring-field", "Hello");

  auto edit = std::make_unique<Wt::WLineEdit>();

  Wt::Form::WFormDelegate<Wt::WString> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "wstring-field", edit.get());

  BOOST_TEST(formModel->valueText("wstring-field").empty());
  BOOST_CHECK(formModel->value("wstring-field").type() == typeid(Wt::WString));

  BOOST_TEST(edit->valueText().empty());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WString_updateViewValue )
{
  // Testing that the value in the view gets correctly updated
  // and that the value in the model remains unchanged.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("wstring-field");
  formModel->setValue("wstring-field", Wt::WString("Hello"));

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("Hi");

  Wt::Form::WFormDelegate<Wt::WString> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "wstring-field", edit.get());

  BOOST_TEST(edit->valueText() == "Hello");

  BOOST_TEST(formModel->valueText("wstring-field") == "Hello");
  BOOST_CHECK(formModel->value("wstring-field").type() == typeid(Wt::WString));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WString_updateViewValue_empty )
{
  // Testing that an empty value in the model will result in clearing the value in the view
  // and that the value in the model remains unchanged.
  // Note that the type of the value in the model isn't checked here, because it's essentially "null".

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("wstring-field");

  auto edit = std::make_unique<Wt::WLineEdit>();
  edit->setValueText("Hi");

  Wt::Form::WFormDelegate<Wt::WString> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "wstring-field", edit.get());

  BOOST_TEST(edit->valueText().empty());

  BOOST_TEST(formModel->valueText("wstring-field").empty());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_createFormWidget )
{
  // Testing that createFormWidget returns the expected widget.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  auto widget = formDelegate.createFormWidget();

  BOOST_TEST(dynamic_cast<Wt::WDateEdit*>(widget.get()));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WDate_createValidator)
{
  // Testing that createValidator returns the expected validator.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<Wt::WDate> formDelegate;
  auto validator = formDelegate.createValidator();

  BOOST_TEST(dynamic_cast<Wt::WDateValidator*>(validator.get()));
}

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

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_createFormWidget )
{
  // Testing that createFormWidget returns the expected widget.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  auto widget = formDelegate.createFormWidget();

  BOOST_TEST(dynamic_cast<Wt::WTimeEdit*>(widget.get()));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_WTime_createValidator)
{
  // Testing that createValidator returns the expected validator.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<Wt::WTime> formDelegate;
  auto validator = formDelegate.createValidator();

  BOOST_TEST(dynamic_cast<Wt::WTimeValidator*>(validator.get()));
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

BOOST_AUTO_TEST_CASE( WFormDelegate_bool_createFormWidget )
{
  // Testing that createFormWidget returns the expected widget.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<bool> formDelegate;
  auto widget = formDelegate.createFormWidget();

  BOOST_TEST(dynamic_cast<Wt::WCheckBox*>(widget.get()));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_bool_createValidator )
{
  // Testing that there is no default validator for bool objects.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::Form::WFormDelegate<bool> formDelegate;
  BOOST_TEST(!formDelegate.createValidator());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_bool_updateModelValue )
{
  // Testing that the value in the model gets correctly updated
  // and that the value in the view remains unchanged.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("bool-field");
  formModel->setValue("bool-field", false);

  auto checkBox = std::make_unique<Wt::WCheckBox>();
  checkBox->setChecked(true);

  Wt::Form::WFormDelegate<bool> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "bool-field", checkBox.get());

  BOOST_CHECK(formModel->value("bool-field").type() == typeid(bool));
  BOOST_TEST(Wt::cpp17::any_cast<bool>(formModel->value("bool-field")));

  BOOST_TEST(checkBox->isChecked());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_bool_updateModelValue_unexpected_widget )
{
  // Testing that the value in the model remains unchanged if we're passing an unexpected
  // widget to updateModelValue (i.e.: no WCheckBox).

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("bool-field");
  formModel->setValue("bool-field", false);

  auto edit = std::make_unique<Wt::WLineEdit>();

  Wt::Form::WFormDelegate<bool> formDelegate;
  formDelegate.updateModelValue(formModel.get(), "bool-field", edit.get());

  BOOST_CHECK(formModel->value("bool-field").type() == typeid(bool));
  BOOST_TEST(!Wt::cpp17::any_cast<bool>(formModel->value("bool-field")));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_bool_updateViewValue )
{
  // Testing that the value in the view gets correctly updated
  // and that the value in the model remains unchanged.

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("bool-field");
  formModel->setValue("bool-field", false);

  auto checkBox = std::make_unique<Wt::WCheckBox>();
  checkBox->setChecked(true);

  Wt::Form::WFormDelegate<bool> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "bool-field", checkBox.get());

  BOOST_TEST(!checkBox->isChecked());

  BOOST_CHECK(formModel->value("bool-field").type() == typeid(bool));
  BOOST_TEST(!Wt::cpp17::any_cast<bool>(formModel->value("bool-field")));
}

BOOST_AUTO_TEST_CASE( WFormDelegate_bool_updateViewValue_unexpected_widget )
{
  // Testing that the value in the view remains unchanged if we're passing an unexpected
  // widget to updateViewValue (i.e.: no WCheckBox).

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("bool-field");
  formModel->setValue("bool-field", false);

  auto edit = std::make_unique<Wt::WLineEdit>();

  Wt::Form::WFormDelegate<bool> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "bool-field", edit.get());

  BOOST_TEST(edit->valueText().empty());
}

BOOST_AUTO_TEST_CASE( WFormDelegate_bool_updateViewValue_unexpected_type )
{
  // Testing that the value in the view is set to false if the model contains a different
  // type for the value (i.e.: no bool).

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  auto formModel = std::make_unique<Wt::WFormModel>();
  formModel->addField("bool-field");
  formModel->setValue("bool-field", "Not a boolean");

  auto checkBox = std::make_unique<Wt::WCheckBox>();
  checkBox->setChecked(true);

  Wt::Form::WFormDelegate<bool> formDelegate;
  formDelegate.updateViewValue(formModel.get(), "bool-field", checkBox.get());

  BOOST_TEST(!checkBox->isChecked());
}
