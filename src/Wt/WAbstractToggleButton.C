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
const char *WAbstractToggleButton::UNDETERMINATE_CLICK_SIGNAL = "M_click";

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
  if (stateChanged_ || all) {
    element.setProperty(Wt::PropertyChecked,
			state_ == Checked ? "true" : "false");

    if (!useImageWorkaround())
      element.setProperty(Wt::PropertyIndeterminate,
			  state_ == PartiallyChecked ? "true" : "false");

    stateChanged_ = false;
  }

  const WEnvironment& env = WApplication::instance()->environment();

  EventSignal<> *check = voidEventSignal(CHECKED_SIGNAL, false);
  EventSignal<> *uncheck = voidEventSignal(UNCHECKED_SIGNAL, false);
  EventSignal<> *change = voidEventSignal(CHANGE_SIGNAL, false);
  EventSignal<WMouseEvent> *click = mouseEventSignal(CLICK_SIGNAL, false);

  bool needUpdateClickedSignal =
    (click && click->needUpdate()
     // onchange does not work on IE
     || (env.agentIsIE() && change && change->needUpdate())
     || (check && check->needUpdate())
     || (uncheck && uncheck->needUpdate()));

  WFormWidget::updateDom(element, all);

  if (needUpdateClickedSignal || all) {
    std::string dom = WT_CLASS ".getElement('" + element.id() + "')";
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
      if (env.agentIsIE() && change->isConnected())
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
  }
}

bool WAbstractToggleButton::useImageWorkaround() const
{
  return false;
}

DomElementType WAbstractToggleButton::domElementType() const
{
  if (useImageWorkaround())
    return DomElement_SPAN;
  else {
    WLabel *l = label();

    if (l && l->parent() == this)
      return DomElement_SPAN;
    else
      return DomElement_INPUT;
  }
}

DomElement *WAbstractToggleButton::createDomElement(WApplication *app)
{
  DomElement *result = DomElement::createNew(domElementType());
  setId(result, app);

  DomElement *input = result;

  if (result->type() == DomElement_SPAN) {
    input = DomElement::createNew(DomElement_INPUT);
    input->setName("in" + id());

    if (useImageWorkaround()) {
      DomElement *img = DomElement::createNew(DomElement_IMG);
      img->setId("im" + id());

      std::string src = WApplication::resourcesUrl();

      const WEnvironment& env = app->environment();

      if (env.userAgent().find("Mac OS X") != std::string::npos)
	src += "indeterminate-macosx.png";
      else if (env.agentIsOpera())
	src += "indeterminate-opera.png";
      else if (env.userAgent().find("Windows") != std::string::npos)
	src += "indeterminate-windows.png"; 
      else
	src += "indeterminate-linux.png";

      img->setProperty(Wt::PropertySrc, fixRelativeUrl(src));
      img->setProperty(Wt::PropertyClass, "Wt-indeterminate");

      EventSignal<> *imgClick
	= voidEventSignal(UNDETERMINATE_CLICK_SIGNAL, true);
      img->setEventSignal("click", *imgClick);
      imgClick->updateOk();

      if (state_ == PartiallyChecked)
	input->setProperty(Wt::PropertyStyleDisplay, "none");
      else
	img->setProperty(Wt::PropertyStyleDisplay, "none");

      result->addChild(img);
    }
  }

  updateDom(*input, true);

  if (result != input) {
    result->addChild(input);

    WLabel *l = label();
  
    if (l && l->parent() == this)
      result->addChild(((WWebWidget *)l)->createDomElement(app));
  }

  return result;
}

void WAbstractToggleButton::getDomChanges(std::vector<DomElement *>& result,
					  WApplication *app)
{
  DomElementType type = domElementType();

  if (type == DomElement_SPAN) {
    DomElement *input
      = DomElement::getForUpdate("in" + id(), DomElement_INPUT);

    if (useImageWorkaround()) {
      EventSignal<> *imgClick = voidEventSignal(UNDETERMINATE_CLICK_SIGNAL,
						true);

      if (stateChanged_ || imgClick->needUpdate()) {
	DomElement *img
	  = DomElement::getForUpdate("im" + id(), DomElement_IMG);

	if (stateChanged_) {
	  img->setProperty(Wt::PropertyStyleDisplay,
			   state_ == PartiallyChecked ? "inline" : "none");
	  input->setProperty(Wt::PropertyStyleDisplay,
			     state_ == PartiallyChecked ? "none" : "inline");
	}

	if (imgClick->needUpdate()) {
	  img->setEventSignal("click", *imgClick);
	  imgClick->updateOk();
	}

	result.push_back(img);
      }
    }

    updateDom(*input, false);
    result.push_back(input);
  } else {
    DomElement *e = DomElement::getForUpdate(this, domElementType());
    updateDom(*e, false);
    result.push_back(e);
  }
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

  if (!formData.values.empty())
    if (formData.values[0] == "indeterminate")
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

std::string WAbstractToggleButton::formName() const
{
  if (domElementType() == DomElement_SPAN && !useImageWorkaround())
    return "in" + id();
  else
    return WFormWidget::formName();
}

}
