/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WContainerWidget>

#include "Label.h"

Label::Label(const WString& text, WContainerWidget *parent)
  : WText(text, parent)
{ 
  setStyleClass(L"label");
  parent->setContentAlignment(AlignRight);
}
