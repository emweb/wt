/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Ext/AbstractToggleButton"
#include "Wt/WAbstractToggleButton"

#include "DomElement.h"

namespace Wt {
  namespace Ext {

AbstractToggleButton::AbstractToggleButton(WAbstractToggleButton *wtWidget,
					   const WString& text,
					   WContainerWidget *parent)
  : FormField(parent),
    wtWidget_(wtWidget),
    text_(text)
{
  addOrphan(wtWidget_);
}

EventSignal<>& AbstractToggleButton::checked()
{
  return wtWidget_->checked();
}

EventSignal<>& AbstractToggleButton::unChecked()
{
  return wtWidget_->unChecked();
}

WFormWidget *AbstractToggleButton::formWidget() const
{
  return wtWidget_;
}

void AbstractToggleButton::setText(const WString& text)
{
  text_ = text;
}

bool AbstractToggleButton::isChecked() const
{
  return wtWidget_->isChecked();
}

void AbstractToggleButton::setChecked(bool how)
{
  wtWidget_->setChecked(how);
}

void AbstractToggleButton::setChecked()
{
  wtWidget_->setChecked();
}

void AbstractToggleButton::setUnChecked()
{
  wtWidget_->setUnChecked();
}

void AbstractToggleButton::useAsTableViewEditor()
{
  wtWidget_->setFormObject(false);
}

std::string AbstractToggleButton::createJS(DomElement *inContainer)
{
  std::stringstream result;
  result << elVar() << " = new Ext.form." << getExtName()
	 << "(" << configStruct() << ");";

  applyToWidget(wtWidget_, result, inContainer);

  return result.str();
}

bool AbstractToggleButton::applySelfCss() const
{
  return false;
}

void AbstractToggleButton::createConfig(std::ostream& config)
{
  config << ",boxLabel:" << text_.jsStringLiteral()
	 << ",checked:" << (isChecked() ? "true" : "false");

  FormField::createConfig(config);
}

  }
}
