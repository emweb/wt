/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WCheckBox"
#include "DomElement.h"

namespace Wt {

WCheckBox::WCheckBox(WContainerWidget *parent)
  : WAbstractToggleButton(parent)
{
  setFormObject(true);
}

WCheckBox::WCheckBox(const WString& text, WContainerWidget *parent)
  : WAbstractToggleButton(text, parent)
{
  setFormObject(true);
}

WCheckBox::WCheckBox(bool withLabel, const WString& text,
		     WContainerWidget *parent)
  : WAbstractToggleButton(withLabel, text, parent)
{
  setFormObject(true);
}

void WCheckBox::updateDom(DomElement& element, bool all)
{
  if (all)
    element.setAttribute("type", "checkbox");

  WAbstractToggleButton::updateDom(element, all);
}

DomElementType WCheckBox::domElementType() const
{
  return DomElement_INPUT;
}

}
