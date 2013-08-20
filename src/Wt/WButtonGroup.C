/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WebUtils.h"

#include "Wt/WButtonGroup"
#include "Wt/WRadioButton"

namespace Wt {

WButtonGroup::WButtonGroup(WObject* parent)
  : WObject(parent),
    checkedChangedConnected_(false)
{ }

WButtonGroup::~WButtonGroup()
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    buttons_[i].button->setGroup(0);
}

void WButtonGroup::addButton(WRadioButton *button, int id)
{
  Button b;
  b.button = button;
  b.id = id != -1 ? id : generateId();
  buttons_.push_back(b);

  button->setGroup(this);

  if (checkedChangedConnected_)
    button->changed().connect(this, &WButtonGroup::onButtonChange);
}

void WButtonGroup::removeButton(WRadioButton *button)
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].button == button) {
      buttons_.erase(buttons_.begin() + i);
      button->setGroup(0);
      return;
    }
}

WRadioButton *WButtonGroup::button(int id) const
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].id == id)
      return buttons_[i].button;

  return 0;
}

int WButtonGroup::id(WRadioButton *button) const
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].button == button)
      return buttons_[i].id;

  return -1;
}

std::vector<WRadioButton *> WButtonGroup::buttons() const
{
  std::vector<WRadioButton *> buttons;

  for (unsigned i = 0; i < buttons_.size(); ++i)
    buttons.push_back(buttons_[i].button);

  return buttons;
}

int WButtonGroup::count() const
{
  return buttons_.size();
}

int WButtonGroup::checkedId() const
{
  int idx = selectedButtonIndex();

  return idx == -1 ? -1 : buttons_[idx].id;
}

void WButtonGroup::setCheckedButton(WRadioButton *button)
{
  for (unsigned i = 0; i < buttons_.size(); ++i) {
    WRadioButton *b = buttons_[i].button;
    if (b == button && !b->isChecked())
      b->setChecked(true);
    else if (b != button && b->isChecked())
      b->setChecked(false);
  }
}

WRadioButton *WButtonGroup::checkedButton() const
{
  int idx = selectedButtonIndex();

  return idx != -1 ? buttons_[idx].button : 0;
}

void WButtonGroup::setSelectedButtonIndex(int idx)
{
  setCheckedButton(idx != -1 ? buttons_[idx].button : 0);
}

int WButtonGroup::selectedButtonIndex() const
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].button->isChecked())
      return i;
  return -1;
}

#ifndef WT_TARGET_JAVA
WRadioButton* WButtonGroup::selectedButton() const
{
  return checkedButton();
}
#endif // WT_TARGET_JAVA

void WButtonGroup::setFormData(const FormData& formData)
{
  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];

    for (unsigned i = 0; i < buttons_.size(); ++i) {
      if (value == buttons_[i].button->id()) {
	if (buttons_[i].button->stateChanged_)
	  return;

	uncheckOthers(buttons_[i].button);
	buttons_[i].button->state_ = Checked;

	return;
      }
    }
  } else {
    /*
     * none checked (form submit) or always for ajax. In any case
     * we don't do anything, since none checked can only be if
     * there were actually none checked to start with ?
     */
  }
}

void WButtonGroup::uncheckOthers(WRadioButton *button)
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].button != button)
      buttons_[i].button->state_ = Unchecked;
}

int WButtonGroup::generateId() const
{
  int id = 0;

  for (unsigned i = 0; i < buttons_.size(); ++i)
    id = std::max(buttons_[i].id + 1, id);

  return id;
}

Signal<WRadioButton *>& WButtonGroup::checkedChanged()
{
  if (!checkedChangedConnected_) {
    checkedChangedConnected_ = true;

    for (unsigned i = 0; i < buttons_.size(); ++i)
      buttons_[i].button->changed()
	.connect(this, &WButtonGroup::onButtonChange);
  }

  return checkedChanged_;
}

void WButtonGroup::onButtonChange()
{
  checkedChanged_.emit(checkedButton());
}

}
