/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Form/WFormDelegate.h"

#include <Wt/WDateEdit.h>
#include <Wt/WDateValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLogger.h>

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
  }
}
