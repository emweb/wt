/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WRadioButton"
#include "DomElement.h"
#include "Wt/WButtonGroup"
#include "CgiParser.h"
#include "Utils.h"

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

WRadioButton::WRadioButton(bool withLabel, const WString& text,
			   WContainerWidget *parent)
  : WAbstractToggleButton(withLabel, text, parent),
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
      element.setAttribute("name", buttonGroup_->formName());
      element.setAttribute("value", formName());
    } else
      element.setAttribute("name", formName());
  }

  WAbstractToggleButton::updateDom(element, all);
}

void WRadioButton::getFormObjects(std::vector<WObject *>& formObjects)
{
  if (buttonGroup_) {
    int i = Utils::indexOf<WObject *>(formObjects, buttonGroup_);

    if (i == -1)
      formObjects.push_back(buttonGroup_);
  }

  formObjects.push_back(this);
}

DomElementType WRadioButton::domElementType() const
{
  return DomElement_INPUT;
}

void WRadioButton::setGroup(WButtonGroup *group)
{
  buttonGroup_ = group;
}

void WRadioButton::setFormData(CgiEntry *entry)
{
  if (entry->value() == formName()) {
    buttonGroup_->uncheckOthers(this);
    checked_ = true;
  } else
    if (!buttonGroup_)
      WAbstractToggleButton::setFormData(entry);
}

void WRadioButton::setNoFormData()
{
  if (!buttonGroup_)
    WAbstractToggleButton::setNoFormData();
}

}
