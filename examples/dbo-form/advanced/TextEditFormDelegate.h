// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TEXT_EDIT_FORM_DELEGATE_H_
#define TEXT_EDIT_FORM_DELEGATE_H_

#include <Wt/Form/WFormDelegate.h>

class TextEditFormDelegate : public Wt::Form::WFormDelegate<Wt::WString, void>
{
public:
  TextEditFormDelegate();

  std::unique_ptr<Wt::WWidget> createFormWidget() override;
};

#endif // TEXT_EDIT_FORM_DELEGATE_H_
