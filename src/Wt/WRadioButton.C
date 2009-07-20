/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WRadioButton"
#include "Wt/WButtonGroup"

#include "Utils.h"
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

void WRadioButton::updateDom(DomElement& element, bool all)
{
  if (all) {
    element.setAttribute("type", "radio");

    if (buttonGroup_) {
      element.setAttribute("name", buttonGroup_->id());
      element.setAttribute("value", id());
    }
  }

  WAbstractToggleButton::updateDom(element, all);
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
  if (stateChanged_)
    return;

  if (!formData.values.empty()) {
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
