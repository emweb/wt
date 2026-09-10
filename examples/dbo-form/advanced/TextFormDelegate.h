// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TEST_FORM_DELEGATE_H_
#define TEST_FORM_DELEGATE_H_

#include <Wt/Form/WFormDelegate.h>

#include "model/CustomSqlTraits.h"

namespace Wt {
  namespace Form {
/*! \brief Form delegate for Text objects */
template<>
class WFormDelegate<Text, void> : public WAbstractFormDelegate
{
public:
  WFormDelegate();

  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;

  void updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};
  }
}
#endif // TEXT_FORM_DELEGATE_H_
