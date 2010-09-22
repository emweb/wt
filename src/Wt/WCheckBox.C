/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCheckBox"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WJavaScript"

#include "DomElement.h"

namespace {
  Wt::JSlot safariWorkaroundJS("function(obj, e) { obj.onchange(); }");
  Wt::JSlot clearOpacityJS("function(obj, e) { obj.style.opacity=''; }");
}

namespace Wt {

WCheckBox::WCheckBox(WContainerWidget *parent)
  : WAbstractToggleButton(parent),
    triState_(false),
    safariWorkaround_(false)
{
  setFormObject(true);
}

WCheckBox::WCheckBox(const WString& text, WContainerWidget *parent)
  : WAbstractToggleButton(text, parent),
    triState_(false),
    safariWorkaround_(false)
{
  setFormObject(true);
}

void WCheckBox::setTristate(bool tristate)
{
  triState_ = tristate;

  if (triState_) {
    if (!supportsIndeterminate(WApplication::instance()->environment()))
      clicked().connect(clearOpacityJS);
    else if (WApplication::instance()->environment().agentIsSafari()
	     && !safariWorkaround_) {
      clicked().connect(safariWorkaroundJS);
      safariWorkaround_ = true;
    }
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
