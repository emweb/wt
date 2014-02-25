/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCheckBox"

#include "Wt/WApplication"
#include "Wt/WEnvironment"

#include "DomElement.h"

namespace Wt {

WCheckBox::WCheckBox(WContainerWidget *parent)
  : WAbstractToggleButton(parent),
    triState_(false)
{
  setFormObject(true);
}

WCheckBox::WCheckBox(const WString& text, WContainerWidget *parent)
  : WAbstractToggleButton(text, parent),
    triState_(false)
{
  setFormObject(true);
}

void WCheckBox::setTristate(bool tristate)
{
  triState_ = tristate;

  if (triState_) {
    if (!supportsIndeterminate(WApplication::instance()->environment()))
      changed().connect("function(obj, e) { obj.style.opacity=''; }");
  }
}

void WCheckBox::setCheckState(CheckState state)
{
  WAbstractToggleButton::setCheckState(state);
}

void WCheckBox::updateInput(DomElement& input, bool all)
{
  if (all)
    input.setAttribute("type", "checkbox");
}

}
