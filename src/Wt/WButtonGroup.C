/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WebUtils.h"

#include "Wt/WButtonGroup.h"
#include "Wt/WRadioButton.h"

namespace Wt {

WButtonGroup::WButtonGroup()
  : checkedChangedConnected_(false)
{ }

WButtonGroup::~WButtonGroup()
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    buttons_[i].button->setGroup(nullptr);
}

void WButtonGroup::addButton(WRadioButton *button, int id)
{
  Button b;
  b.button = button;
  b.id = id != -1 ? id : generateId();
  buttons_.push_back(b);

#ifndef WT_TARGET_JAVA
  button->setGroup(this->shared_from_this());
#else
  button->setGroup(std::shared_ptr<WButtonGroup>(this));
#endif

  if (checkedChangedConnected_)
    button->changed().connect(this, &WButtonGroup::onButtonChange);
}

void WButtonGroup::removeButton(WRadioButton *button)
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].button == button) {
      buttons_.erase(buttons_.begin() + i);
      button->setGroup(nullptr);
      return;
    }
}

WRadioButton *WButtonGroup::button(int id) const
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].id == id)
      return buttons_[i].button;

  return nullptr;
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

  return idx != -1 ? buttons_[idx].button : nullptr;
}

void WButtonGroup::setSelectedButtonIndex(int idx)
{
  setCheckedButton(idx != -1 ? buttons_[idx].button : nullptr);
}

int WButtonGroup::selectedButtonIndex() const
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].button->isChecked())
      return i;
  return -1;
}

void WButtonGroup::setFormData(const FormData& formData)
{
  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];

    for (unsigned i = 0; i < buttons_.size(); ++i) {
      if (value == buttons_[i].button->id()) {
	if (buttons_[i].button->flags_.test
	    (WAbstractToggleButton::BIT_STATE_CHANGED))
	  return;

	uncheckOthers(buttons_[i].button);
	buttons_[i].button->state_ = CheckState::Checked;

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
      buttons_[i].button->state_ = CheckState::Unchecked;
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
