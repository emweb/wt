/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRadioButton.h"
#include "Wt/WButtonGroup.h"

#include "WebUtils.h"
#include "DomElement.h"

namespace Wt {

WRadioButton::WRadioButton()
  : buttonGroup_(nullptr)
{
  setFormObject(true);
}

WRadioButton::WRadioButton(const WString& text)
  : WAbstractToggleButton(text),
    buttonGroup_(nullptr)
{
  setFormObject(true);
}

WRadioButton::~WRadioButton()
{
  if (buttonGroup_)
    buttonGroup_->removeButton(this);
}

void WRadioButton::updateInput(DomElement& input, bool all)
{
  if (all) {
    input.setAttribute("type", "radio");

    if (buttonGroup_) {
      input.setAttribute("name", buttonGroup_->id());
      input.setAttribute("value", id());
    }
  }
}

void WRadioButton::getFormObjects(FormObjectsMap& formObjects)
{
  if (buttonGroup_)
    formObjects[buttonGroup_->id()] = buttonGroup_.get();

  WAbstractToggleButton::getFormObjects(formObjects);
}

void WRadioButton::setGroup(std::shared_ptr<WButtonGroup> group)
{
  buttonGroup_ = group;
}

void WRadioButton::setFormData(const FormData& formData)
{
  if (flags_.test(BIT_STATE_CHANGED) || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];

    if (value == id()) {
      if (buttonGroup_) {
	buttonGroup_->uncheckOthers(this);
	state_ = CheckState::Checked;
      }
    } else
      if (!buttonGroup_)
	WAbstractToggleButton::setFormData(formData);
  } else
    if (!buttonGroup_)
      WAbstractToggleButton::setFormData(formData);
}

}
