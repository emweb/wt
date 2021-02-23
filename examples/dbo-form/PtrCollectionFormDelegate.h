// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef PTR_COLLECTION_FORM_DELEGATE_H_
#define PTR_COLLECTION_FORM_DELEGATE_H_

#include <Wt/Form/WAbstractFormDelegate.h>

class PtrCollectionFormDelegate : public Wt::Form::WAbstractFormDelegate
{
public:
  explicit PtrCollectionFormDelegate(Wt::Dbo::Session& session);

  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;

  void updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;

private:
  Wt::Dbo::Session& session_;
};

#endif // PTR_COLLECTION_FORM_DELEGATE_H_
