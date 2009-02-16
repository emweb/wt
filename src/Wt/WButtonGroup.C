/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "CgiParser.h"
#include "Utils.h"

#include "Wt/WButtonGroup"
#include "Wt/WRadioButton"

namespace Wt {

WButtonGroup::WButtonGroup(WObject* parent)
    :WObject(parent)
{ }

WButtonGroup::~WButtonGroup()
{
  for (unsigned i = 0; i < buttons_.size(); ++i) {
    buttons_[i]->setGroup(0);
  }
}

void WButtonGroup::addButton(WRadioButton *button)
{
  buttons_.push_back(button);
  button->setGroup(this);
}

void WButtonGroup::removeButton(WRadioButton *button)
{
  Utils::erase(buttons_, button);
  button->setGroup(0);
}

void WButtonGroup::uncheckOthers(WRadioButton *button)
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i] != button)
      buttons_[i]->checked_ = false;
}

void WButtonGroup::setSelectedButtonIndex(int idx)
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (((int)i == idx) && !buttons_[i]->isChecked())
      buttons_[i]->setChecked(true);
    else if (((int)i != idx) && buttons_[i]->isChecked())
      buttons_[i]->setChecked(false);
}

int WButtonGroup::selectedButtonIndex() const
{
  for (unsigned i = 0; i< buttons_.size(); ++i)
    if (buttons_[i]->checked_)
      return i;
  return -1;
}

WRadioButton* WButtonGroup::selectedButton()
{
  for (unsigned int i = 0; i< buttons_.size(); ++i)
    if (buttons_[i]->checked_)
      return buttons_[i];
  return 0;
}

void WButtonGroup::setFormData(CgiEntry *entry)
{
  for (unsigned i = 0; i < buttons_.size(); ++i) {
    if (entry->value() == buttons_[i]->formName()) {
      uncheckOthers(buttons_[i]);
      buttons_[i]->checked_ = true;
    }
  }
}

int WButtonGroup::count() const
{
  return buttons_.size();
}

void WButtonGroup::setNoFormData()
{
  /*
   * none checked (form submit) or aways for ajax. In any case
   * we don't do anything, since none checked can only be if
   * there were actually none checked to start with ?
   */
}

}
