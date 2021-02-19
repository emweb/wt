/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Form/WAbstractFormDelegate.h"

#include <Wt/WAbstractToggleButton.h>
#include <Wt/WFormWidget.h>

namespace Wt {
  namespace Form {
WAbstractFormDelegate::WAbstractFormDelegate()
{
}

std::shared_ptr<Wt::WValidator> WAbstractFormDelegate::createValidator()
{
  return nullptr;
}

void WAbstractFormDelegate::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  model->setValue(field, edit->valueText());
}

bool WAbstractFormDelegate::updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
{
  return false;
}

void WAbstractFormDelegate::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit)
{
  edit->setValueText(model->valueText(field));
}

bool WAbstractFormDelegate::updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit)
{
  return false;
}
  }
}

