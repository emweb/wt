/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Form/WFormDelegate.h"

#include <Wt/WCheckBox.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDateTime.h>
#include <Wt/WDateValidator.h>
#include <Wt/WDoubleValidator.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLocale.h>
#include <Wt/WLogger.h>
#include <Wt/WTimeEdit.h>
#include <Wt/WTimeValidator.h>

namespace Wt {
LOGGER("WFormDelegate");
}

namespace Wt {
  namespace Form {
WFormDelegate<Wt::WString, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<Wt::WString, void>::createFormWidget()
{
  return std::make_unique<Wt::WLineEdit>();
}

WFormDelegate<std::string, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<std::string, void>::createFormWidget()
{
  return std::make_unique<Wt::WLineEdit>();
}

void WFormDelegate<std::string, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  model->setValue(field, edit->valueText().toUTF8());
}

WFormDelegate<Wt::WDate, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<Wt::WDate, void>::createFormWidget()
{
  return std::make_unique<Wt::WDateEdit>();
}

std::shared_ptr<Wt::WValidator> WFormDelegate<Wt::WDate, void>::createValidator()
{
  return std::make_shared<Wt::WDateValidator>();
}

void WFormDelegate<Wt::WDate, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Wt::WDateEdit *dateEdit = dynamic_cast<Wt::WDateEdit *>(edit);
  if (dateEdit) {
    model->setValue(field, dateEdit->date());
  } else {
    LOG_ERROR("Could not cast edit to WDateEdit!");
  }
}

WFormDelegate<Wt::WTime, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<Wt::WTime, void>::createFormWidget()
{
  return std::make_unique<Wt::WTimeEdit>();
}

std::shared_ptr<Wt::WValidator> WFormDelegate<Wt::WTime, void>::createValidator()
{
  return std::make_shared<Wt::WTimeValidator>();
}

void WFormDelegate<Wt::WTime, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Wt::WTimeEdit *timeEdit = dynamic_cast<Wt::WTimeEdit *>(edit);
  if (timeEdit) {
    model->setValue(field, timeEdit->time());
  } else {
    LOG_ERROR("Could not cast edit to WTimeEdit!");
  }
}

WFormDelegate<Wt::WDateTime, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<Wt::WDateTime, void>::createFormWidget()
{
  return std::make_unique<Wt::WLineEdit>();
}

void WFormDelegate<Wt::WDateTime, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  if (!edit->valueText().empty()) {
    Wt::WDateTime value = Wt::WDateTime::fromString(edit->valueText(), Wt::WLocale::currentLocale().dateTimeFormat());
    model->setValue(field, value);
  }
}

WFormDelegate<bool,void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<bool, void>::createFormWidget()
{
  return std::make_unique<Wt::WCheckBox>();
}

void WFormDelegate<bool, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Wt::WCheckBox *box = dynamic_cast<Wt::WCheckBox *>(edit);
  if (box) {
    model->setValue(field, box->isChecked());
  } else {
    LOG_ERROR("Could not cast edit to WCheckBox!");
  }
}

void WFormDelegate<bool, void>::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Wt::WCheckBox *box = dynamic_cast<Wt::WCheckBox *>(edit);
  if (box) {
    Wt::cpp17::any v = model->value(field);

    bool value = false;
    if (Wt::cpp17::any_has_value(v)) {
      try {
        value = Wt::cpp17::any_cast<bool>(v);
      } catch (std::exception& e) {
        LOG_ERROR("Could not convert value to bool: " << e.what());
      }
    }

    box->setChecked(value);
  } else {
    LOG_ERROR("Could not cast edit to WCheckBox!");
  }
}

WFormDelegate<int, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<int, void>::createFormWidget()
{
  return std::make_unique<Wt::WLineEdit>();
}

std::shared_ptr<Wt::WValidator> WFormDelegate<int, void>::createValidator()
{
  return std::make_shared<Wt::WIntValidator>();
}

void WFormDelegate<int, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  if (!edit->valueText().empty()) {
    int value = 0;
    try {
      value = Wt::WLocale::currentLocale().toInt(edit->valueText());
    } catch (std::exception& e) {
      LOG_ERROR("Could not convert '" << edit->valueText() << "' to integer: " << e.what());

      std::shared_ptr<Wt::WValidator> validator = edit->validator();
      if (validator) {
        model->setValue(field, validator->validate(edit->valueText()));
      }

      return;
    }
    model->setValue(field, value);
  } else {
    model->setValue(field, 0);
  }
}

void WFormDelegate<int, void>::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Wt::cpp17::any v = model->value(field);
  if (v.type() != typeid(Wt::WValidator::Result)) {
    edit->setValueText(Wt::asString(v));
  }
}

WFormDelegate<double, void>::WFormDelegate()
  : WAbstractFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> WFormDelegate<double, void>::createFormWidget()
{
  return std::make_unique<Wt::WLineEdit>();
}

std::shared_ptr<Wt::WValidator> WFormDelegate<double, void>::createValidator()
{
  return std::make_shared<Wt::WDoubleValidator>();
}

void WFormDelegate<double, void>::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  if (!edit->valueText().empty()) {
    double value = 0.0;
    try {
      value = Wt::WLocale::currentLocale().toDouble(edit->valueText());
    } catch (std::exception& e) {
      LOG_ERROR("Could not convert '" << edit->valueText() << "' to double: " << e.what());

      std::shared_ptr<Wt::WValidator> validator = edit->validator();
      if (validator) {
        model->setValue(field, validator->validate(edit->valueText()));
      }

      return;
    }
    model->setValue(field, value);
  } else {
    model->setValue(field, 0.0);
  }
}

void WFormDelegate<double, void>::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  Wt::cpp17::any v = model->value(field);
  if (v.type() != typeid(Wt::WValidator::Result)) {
    edit->setValueText(Wt::asString(v));
  }
}
  }
}
