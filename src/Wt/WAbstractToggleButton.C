/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractToggleButton"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WLabel"

#include "DomElement.h"

namespace Wt {

const char *WAbstractToggleButton::CHECKED_SIGNAL = "M_checked";
const char *WAbstractToggleButton::UNCHECKED_SIGNAL = "M_unchecked";

WAbstractToggleButton::WAbstractToggleButton(WContainerWidget *parent)
  : WFormWidget(parent),
    checked_(false),
    checkedChanged_(false)
{
  WLabel *label = new WLabel(parent);
  label->setBuddy(this);
}

WAbstractToggleButton::WAbstractToggleButton(const WString& text,
					     WContainerWidget *parent)
  : WFormWidget(parent),
    checked_(false),
    checkedChanged_(false)
{ 
  WLabel *label = new WLabel(text, parent);
  label->setBuddy(this);
}

WAbstractToggleButton::WAbstractToggleButton(bool withLabel,
					     const WString& text,
					     WContainerWidget *parent)
  : WFormWidget(parent),
    checked_(false),
    checkedChanged_(false)
{
  if (withLabel) {
    WLabel *label = new WLabel(text, parent);
    label->setBuddy(this);
  }
}

#ifndef WT_TARGET_JAVA
WStatelessSlot *WAbstractToggleButton::getStateless(Method method)
{
  void (WAbstractToggleButton::*setC)() = &WAbstractToggleButton::setChecked;

  if (method == static_cast<WObject::Method>(setC))
    return implementStateless(setC, &WAbstractToggleButton::undoSetChecked);
  else if (method == static_cast<WObject::Method>
	   (&WAbstractToggleButton::setUnChecked))
    return implementStateless(&WAbstractToggleButton::setUnChecked,
			      &WAbstractToggleButton::undoSetUnChecked);
  else
    return WFormWidget::getStateless(method);
}
#endif

EventSignal<>& WAbstractToggleButton::checked()
{
  return *voidEventSignal(CHECKED_SIGNAL, true);
}

EventSignal<>& WAbstractToggleButton::unChecked()
{
  return *voidEventSignal(UNCHECKED_SIGNAL, true);
}

void WAbstractToggleButton::load()
{
  WLabel *l = label();
  if (l) {
    WWidget *w = l->parent();
    if (!w) {
      WContainerWidget *p = dynamic_cast<WContainerWidget *>(parent());
      if (p)
	p->insertWidget(p->indexOf(this) + 1, l);
    }
  }
  WFormWidget::load();
}

void WAbstractToggleButton::setText(const WString& text)
{
  if (label())
    label()->setText(text);
}

const WString WAbstractToggleButton::text() const
{
  if (label())
    return label()->text();
  else
    return WString();
}

void WAbstractToggleButton::setChecked(bool how)
{
  if (canOptimizeUpdates() && checked_ == how)
    return;

  checked_ = how;
  checkedChanged_ = true;

  /*
  if (how)
    checked().emit();
  else
    unChecked().emit();
  */

  repaint(RepaintPropertyIEMobile);
}

void WAbstractToggleButton::setChecked()
{
  wasChecked_ = checked_;
  setChecked(true);
}

void WAbstractToggleButton::setUnChecked()
{
  wasChecked_ = checked_;
  setChecked(false);
}

void WAbstractToggleButton::undoSetChecked()
{
  setChecked(checked_);
}

void WAbstractToggleButton::undoSetUnChecked()
{
  undoSetChecked();
}

void WAbstractToggleButton::updateDom(DomElement& element, bool all)
{
  if (checkedChanged_ || all) {
    element.setProperty(Wt::PropertyChecked, checked_ ? "true" : "false");
    checkedChanged_ = false;
  }

  const WEnvironment& env = WApplication::instance()->environment();

  EventSignal<> *check = voidEventSignal(CHECKED_SIGNAL, false);
  EventSignal<> *uncheck = voidEventSignal(UNCHECKED_SIGNAL, false);
  EventSignal<> *change = voidEventSignal(CHANGE_SIGNAL, false);
  EventSignal<WMouseEvent> *click = mouseEventSignal(CLICK_SIGNAL, false);

  bool needUpdateClickedSignal =
    (click && click->needUpdate()
     || (env.agentIE() && change && change->needUpdate())
        // onchange does not work on IE
     || (check && check->needUpdate())
     || (uncheck && uncheck->needUpdate()));

  WFormWidget::updateDom(element, all);

  if (needUpdateClickedSignal || all) {
    DomElement *e = DomElement::getForUpdate(this, DomElement_INPUT);

    std::vector<DomElement::EventAction> actions;

    if (check) {
      if (check->isConnected())
	actions.push_back
	  (DomElement::EventAction(e->createReference() + ".checked",
				   check->javaScript(),
				   check->encodeCmd(),
				   check->isExposedSignal()));
      check->updateOk();
    }

    if (uncheck) {
      if (uncheck->isConnected())
	actions.push_back
	  (DomElement::EventAction("!" + e->createReference() + ".checked",
				   uncheck->javaScript(),
				   uncheck->encodeCmd(),
				   uncheck->isExposedSignal()));
      uncheck->updateOk();
    }

    if (change) {
      if (env.agentIE() && change->isConnected())
	actions.push_back
	  (DomElement::EventAction(std::string(),
				   change->javaScript(),
				   change->encodeCmd(),
				   change->isExposedSignal()));
      change->updateOk();
    }

    if (click) {
      if (click->isConnected())
	actions.push_back
	  (DomElement::EventAction(std::string(),
				   click->javaScript(),
				   click->encodeCmd(),
				   click->isExposedSignal()));
      click->updateOk();
    }

    if (!(all && actions.empty()))
      element.setEvent("click", actions);

    delete e;
  }
}

void WAbstractToggleButton::propagateRenderOk(bool deep)
{
  checkedChanged_ = false;

  EventSignal<> *check = voidEventSignal(CHECKED_SIGNAL, false);
  if (check)
    check->updateOk();

  EventSignal<> *uncheck = voidEventSignal(UNCHECKED_SIGNAL, false);
  if (uncheck)
    uncheck->updateOk();

  WInteractWidget::propagateRenderOk(deep);
}

void WAbstractToggleButton::setFormData(const FormData& formData)
{
  if (!formData.values.empty())
    checked_ = formData.values[0] != "0";
  else
    if (isEnabled() && isVisible())
      checked_ = false;
}

}
