/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRadioButton"
#include "Wt/WButtonGroup"

#include "WebUtils.h"
#include "DomElement.h"

namespace Wt {

WRadioButton::WRadioButton(WContainerWidget *parent)
  : WAbstractToggleButton(parent),
    buttonGroup_(0)
{
  setFormObject(true);
}

WRadioButton::WRadioButton(const WString& text, WContainerWidget *parent)
  : WAbstractToggleButton(text, parent),
    buttonGroup_(0)
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
    formObjects[buttonGroup_->id()] = buttonGroup_;

  WAbstractToggleButton::getFormObjects(formObjects);
}

void WRadioButton::setGroup(WButtonGroup *group)
{
  buttonGroup_ = group;
}

void WRadioButton::setFormData(const FormData& formData)
{
  if (stateChanged_ || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];

    if (value == id()) {
      if (buttonGroup_) {
	buttonGroup_->uncheckOthers(this);
	state_ = Checked;
      }
    } else
      if (!buttonGroup_)
	WAbstractToggleButton::setFormData(formData);
  } else
    if (!buttonGroup_)
      WAbstractToggleButton::setFormData(formData);
}

}
