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
#include "Utils.h"

namespace Wt {

const char *WAbstractToggleButton::CHECKED_SIGNAL = "M_checked";
const char *WAbstractToggleButton::UNCHECKED_SIGNAL = "M_unchecked";

WAbstractToggleButton::WAbstractToggleButton(WContainerWidget *parent)
  : WFormWidget(parent),
    state_(Unchecked),
    stateChanged_(false)
{ }

WAbstractToggleButton::WAbstractToggleButton(const WString& text,
					     WContainerWidget *parent)
  : WFormWidget(parent),
    state_(Unchecked),
    stateChanged_(false)
{ 
  WLabel *label = new WLabel(text);
  label->setBuddy(this);
  addChild(label);
}

WAbstractToggleButton::~WAbstractToggleButton()
{ }

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

void WAbstractToggleButton::setText(const WString& text)
{
  WLabel *l = label();

  if (!l) {
    l = new WLabel(text);
    l->setBuddy(this);
    addChild(l);
  }

  l->setText(text);
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
  setCheckState(how ? Checked : Unchecked);
}

void WAbstractToggleButton::setCheckState(CheckState state)
{
  if (canOptimizeUpdates() && state == state_)
    return;

  state_ = state;
  stateChanged_ = true;
  repaint(RepaintPropertyIEMobile);
}

void WAbstractToggleButton::setChecked()
{
  prevState_ = state_;
  setChecked(true);
}

void WAbstractToggleButton::setUnChecked()
{
  prevState_ = state_;
  setChecked(false);
}

void WAbstractToggleButton::undoSetChecked()
{
  setCheckState(prevState_);
}

void WAbstractToggleButton::undoSetUnChecked()
{
  undoSetChecked();
}

void WAbstractToggleButton::updateDom(DomElement& element, bool all)
{
  WApplication *app = WApplication::instance();
  const WEnvironment& env = app->environment();

  DomElement *input = 0;
  if (element.type() == DomElement_SPAN) {
    if (all) {
      input = DomElement::createNew(DomElement_INPUT);
      input->setName("in" + id());
    } else
      input = DomElement::getForUpdate("in" + id(), DomElement_INPUT);
  } else
    input = &element;

  if (all)
    updateInput(*input, all);

  EventSignal<> *check = voidEventSignal(CHECKED_SIGNAL, false);
  EventSignal<> *uncheck = voidEventSignal(UNCHECKED_SIGNAL, false);
  EventSignal<> *change = voidEventSignal(CHANGE_SIGNAL, false);
  EventSignal<WMouseEvent> *click = mouseEventSignal(CLICK_SIGNAL, false);

  bool needUpdateClickedSignal =
    ((click && click->needsUpdate(all))
     // onchange does not work on IE
     || (env.agentIsIE() && change && change->needsUpdate(all))
     || (check && check->needsUpdate(all))
     || (uncheck && uncheck->needsUpdate(all)));

  WFormWidget::updateDom(*input, all);

  /*
   * Copy all properties to the exterior element, as they relate to style,
   * etc... We ignore here attributes, see WWebWidget: there seems not to
   * be attributes that sensibly need to be moved.
   *
   * But -- bug #423, disabled and readonly are properties that should be
   * kept on the interior element.
   */
  if (&element != input) {
    element.setProperties(input->properties());
    input->clearProperties();

    std::string v = element.getProperty(Wt::PropertyDisabled);
    if (!v.empty()) {
      input->setProperty(Wt::PropertyDisabled, v);
      element.removeProperty(Wt::PropertyDisabled);
    }

    v = element.getProperty(Wt::PropertyReadOnly);
    if (!v.empty()) {
      input->setProperty(Wt::PropertyReadOnly, v);
      element.removeProperty(Wt::PropertyReadOnly);
    }
  }

  if (stateChanged_ || all) {
    input->setProperty(Wt::PropertyChecked,
		       state_ == Unchecked ? "false" : "true");

    if (supportsIndeterminate(env))
      input->setProperty(Wt::PropertyIndeterminate,
			 state_ == PartiallyChecked ? "true" : "false");
    else
      input->setProperty(Wt::PropertyStyleOpacity,
			 state_ == PartiallyChecked ? "0.5" : "");

    stateChanged_ = false;
  }

  if (needUpdateClickedSignal || all) {
    std::string dom = "o";
    std::vector<DomElement::EventAction> actions;

    if (check) {
      if (check->isConnected())
	actions.push_back
	  (DomElement::EventAction(dom + ".checked",
				   check->javaScript(),
				   check->encodeCmd(),
				   check->isExposedSignal()));
      check->updateOk();
    }

    if (uncheck) {
      if (uncheck->isConnected())
	actions.push_back
	  (DomElement::EventAction("!" + dom + ".checked",
				   uncheck->javaScript(),
				   uncheck->encodeCmd(),
				   uncheck->isExposedSignal()));
      uncheck->updateOk();
    }

    if (change) {
      if (env.agentIsIE() && change->needsUpdate(all))
	actions.push_back
	  (DomElement::EventAction(std::string(),
				   change->javaScript(),
				   change->encodeCmd(),
				   change->isExposedSignal()));
      change->updateOk();
    }

    if (click) {
      if (click->needsUpdate(all))
	actions.push_back
	  (DomElement::EventAction(std::string(),
				   click->javaScript(),
				   click->encodeCmd(),
				   click->isExposedSignal()));
      click->updateOk();
    }

    if (!(all && actions.empty()))
      input->setEvent(CLICK_SIGNAL, actions);
  }

  if (&element != input)
    element.addChild(input);

  if (all) {
    WLabel *l = label();
  
    if (l && l->parent() == this)
      element.addChild(((WWebWidget *)l)->createDomElement(app));
  }
}

DomElementType WAbstractToggleButton::domElementType() const
{
  WLabel *l = label();

  if (l && l->parent() == this)
    return DomElement_SPAN;
  else
    return DomElement_INPUT;
}

void WAbstractToggleButton::propagateRenderOk(bool deep)
{
  stateChanged_ = false;

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
  if (stateChanged_)
    return;

  if (!Utils::isEmpty(formData.values))
    if (formData.values[0] == "i")
      state_ = PartiallyChecked;
    else
      state_ = formData.values[0] != "0" ? Checked : Unchecked;
  else
    if (isEnabled() && isVisible())
      state_ = Unchecked;
}

void WAbstractToggleButton::getFormObjects(FormObjectsMap& formObjects)
{
  formObjects[formName()] = this;
}

bool WAbstractToggleButton::supportsIndeterminate(const WEnvironment& env)
  const
{
  return env.javaScript()
    && (env.agentIsIE()
	|| env.agentIsSafari()
	|| (env.agentIsGecko() && (env.agent() >= WEnvironment::Firefox3_6)));
}

std::string WAbstractToggleButton::formName() const
{
  if (domElementType() == DomElement_SPAN)
    return "in" + id();
  else
    return WFormWidget::formName();
}

}
