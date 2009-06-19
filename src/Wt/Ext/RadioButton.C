/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Ext/RadioButton"
#include "Wt/WRadioButton"
#include "Wt/WButtonGroup"

namespace Wt {
  namespace Ext {

RadioButton::RadioButton(const WString& text, WContainerWidget *parent)
  : AbstractToggleButton(new WRadioButton(), text, parent)
{ }

RadioButton::RadioButton(WContainerWidget *parent)
  : AbstractToggleButton(new WRadioButton(), WString(), parent)
{ }

std::string RadioButton::getExtName() const
{
  return "Radio";
}

WRadioButton *RadioButton::wtRadioButton() const
{
  return dynamic_cast<WRadioButton *>(wtWidget());
}

  }

void WButtonGroup::addButton(Ext::RadioButton *button, int id)
{
  addButton(button->wtRadioButton(), id);
}

void WButtonGroup::removeButton(Ext::RadioButton *button)
{
  removeButton(button->wtRadioButton());
}

}
