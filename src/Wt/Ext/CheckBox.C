/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Ext/CheckBox"
#include "Wt/WCheckBox"

namespace Wt {
  namespace Ext {

CheckBox::CheckBox(const WString& text, WContainerWidget *parent)
  : AbstractToggleButton(new WCheckBox(), text, parent)
{ }

CheckBox::CheckBox(WContainerWidget *parent)
  : AbstractToggleButton(new WCheckBox(), WString(), parent)
{ }

std::string CheckBox::getExtName() const
{
  return "Checkbox";
}

  }
}
