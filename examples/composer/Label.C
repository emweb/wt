/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WContainerWidget.h>

#include "Label.h"

Label::Label(const WString& text, WContainerWidget *parent)
  : WText(text)
{ 
  setStyleClass(L"label");
  parent->setContentAlignment(AlignmentFlag::Right);
}
