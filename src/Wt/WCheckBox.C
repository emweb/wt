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
  Wt::JSlot safariWorkaroundJS("function(obj, e) { obj.onchange(); };");
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

  if (triState_)
    if (needTristateImageWorkaround()) {
      EventSignal<> *imgClick
	= voidEventSignal(UNDETERMINATE_CLICK_SIGNAL, false);
      if (!imgClick) {
	imgClick = voidEventSignal(UNDETERMINATE_CLICK_SIGNAL, true);
	imgClick->connect(SLOT(this, WCheckBox::setUnChecked));
	imgClick->connect(SLOT(this, WCheckBox::gotUndeterminateClick));
      }
    } else if (WApplication::instance()->environment().agentIsSafari()
	       && !safariWorkaround_) {
      clicked().connect(safariWorkaroundJS);
      safariWorkaround_ = true;
    }
}

void WCheckBox::gotUndeterminateClick()
{
  setUnChecked();
  unChecked().emit();
  changed().emit();
}

void WCheckBox::setCheckState(CheckState state)
{
  WAbstractToggleButton::setCheckState(state);
}

bool WCheckBox::useImageWorkaround() const
{
  return triState_ && needTristateImageWorkaround();
}

bool WCheckBox::needTristateImageWorkaround() const
{
  WApplication *app = WApplication::instance();

  bool supportIndeterminate
    = app->environment().javaScript()
    && (app->environment().agentIsIE()
	|| app->environment().agentIsSafari()
	|| (app->environment().agentIsGecko()
	    && app->environment().agent() >= WEnvironment::Firefox3_1b));

  return !supportIndeterminate;
}

void WCheckBox::updateDom(DomElement& element, bool all)
{
  if (all)
    element.setAttribute("type", "checkbox");

  WAbstractToggleButton::updateDom(element, all);
}

}
