/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractToggleButton"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WTheme"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

const char *WAbstractToggleButton::CHECKED_SIGNAL = "M_checked";
const char *WAbstractToggleButton::UNCHECKED_SIGNAL = "M_unchecked";

WAbstractToggleButton::WAbstractToggleButton(WContainerWidget *parent)
  : WFormWidget(parent),
    state_(Unchecked),
    stateChanged_(false),
    textChanged_(false)
{
  text_.format = PlainText;
}

WAbstractToggleButton::WAbstractToggleButton(const WString& text,
					     WContainerWidget *parent)
  : WFormWidget(parent),
    state_(Unchecked),
    stateChanged_(false),
    textChanged_(false)
{ 
  text_.format = PlainText;
  text_.text = text;
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
  if (canOptimizeUpdates() && (text == text_.text))
    return;

  text_.setText(text);
  textChanged_ = true;
  repaint(RepaintSizeAffected);
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
  repaint();
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
  DomElement *span = 0;
  DomElement *label = 0;

  // Already apply theme here because it may determine its organization
  if (all)
    app->theme()->apply(this, element, 1);

  if (element.type() == DomElement_INPUT)
    input = &element;
  else {
    if (all) {
      input = DomElement::createNew(DomElement_INPUT);
      input->setName("in" + id());

      span = DomElement::createNew(DomElement_SPAN);
      span->setName("t" + id());
 
      if (element.type() != DomElement_LABEL) {
	label = DomElement::createNew(DomElement_LABEL);
	label->setName("l" + id());
      }
    } else {
      input = DomElement::getForUpdate("in" + id(), DomElement_INPUT);
      span = DomElement::getForUpdate("t" + id(), DomElement_SPAN);
    }
  }

  if (all)
    updateInput(*input, all);

  EventSignal<> *check = voidEventSignal(CHECKED_SIGNAL, false);
  EventSignal<> *uncheck = voidEventSignal(UNCHECKED_SIGNAL, false);
  EventSignal<> *change = voidEventSignal(CHANGE_SIGNAL, false);
  EventSignal<WMouseEvent> *click = mouseEventSignal(M_CLICK_SIGNAL, false);

  /*
   * We piggy-back the checked and uncheck signals on the change signal.
   *
   * If agent is IE, then we piggy-back the change on the clicked signal.
   */
  bool piggyBackChangeOnClick = env.agentIsIE();

  bool needUpdateChangeSignal =
    (change && change->needsUpdate(all))
    || (check && check->needsUpdate(all))
    || (uncheck && uncheck->needsUpdate(all));

  bool needUpdateClickedSignal =
    (click && click->needsUpdate(all))
     || (piggyBackChangeOnClick && needUpdateChangeSignal);

  WFormWidget::updateDom(*input, all);

  /*
   * Copy all properties to the exterior element, as they relate to style,
   * etc... We ignore here attributes (except for tooltip),
   * see WWebWidget: other attributes need not be moved.
   *
   * But -- bug #423, disabled and readonly are properties that should be
   * kept on the interior element.
   */
  if (&element != input) {
    if (element.properties().find(PropertyClass) != element.properties().end())
      input->addPropertyWord(PropertyClass, element.getProperty(PropertyClass));
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

    v = input->getAttribute("title");
    if (!v.empty())
      element.setAttribute("title", v);
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

  std::vector<DomElement::EventAction> changeActions;

  if (needUpdateChangeSignal
      || (piggyBackChangeOnClick && needUpdateClickedSignal)
      || all) {
    std::string dom = "o";

    if (check) {
      if (check->isConnected())
	changeActions.push_back
	  (DomElement::EventAction(dom + ".checked",
				   check->javaScript(),
				   check->encodeCmd(),
				   check->isExposedSignal()));
      check->updateOk();
    }

    if (uncheck) {
      if (uncheck->isConnected())
	changeActions.push_back
	  (DomElement::EventAction("!" + dom + ".checked",
				   uncheck->javaScript(),
				   uncheck->encodeCmd(),
				   uncheck->isExposedSignal()));
      uncheck->updateOk();
    }

    if (change) {
      if (change->isConnected())
	changeActions.push_back
	  (DomElement::EventAction(std::string(),
				   change->javaScript(),
				   change->encodeCmd(),
				   change->isExposedSignal()));
      change->updateOk();
    }

    if (!piggyBackChangeOnClick) {
      if (!(all && changeActions.empty()))
	input->setEvent("change", changeActions);
    }
  }

  if (needUpdateClickedSignal || all) {
    if (piggyBackChangeOnClick) {
      if (click) {
	changeActions.push_back
	  (DomElement::EventAction(std::string(),
				   click->javaScript(),
				   click->encodeCmd(),
				   click->isExposedSignal()));
	click->updateOk();
      }

      if (!(all && changeActions.empty()))
	input->setEvent(CLICK_SIGNAL, changeActions);
    } else
      if (click)
	updateSignalConnection(*input, *click, CLICK_SIGNAL, all);
  }

  if (span) {
    if (all || textChanged_) {
      span->setProperty(PropertyInnerHTML, text_.formattedText());
      textChanged_ = false;
    }
  }

  if (&element != input) {
    if (label) {
      label->addChild(input);
      label->addChild(span);
      element.addChild(label);
    } else {
      element.addChild(input);
      element.addChild(span);
    }
  }
}

DomElementType WAbstractToggleButton::domElementType() const
{
  if (!text_.text.empty())
    return DomElement_LABEL;
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
  if (stateChanged_ || isReadOnly())
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
  if (domElementType() == DomElement_LABEL)
    return "in" + id();
  else
    return WFormWidget::formName();
}

WT_USTRING WAbstractToggleButton::valueText() const
{
  switch (state_) {
  case Unchecked: return "no";
  case PartiallyChecked: return "maybe";
  default: return "yes";
  }
}

void WAbstractToggleButton::setValueText(const WT_USTRING& text)
{
  if (text == "yes")
    setCheckState(Checked);
  else if (text == "no")
    setCheckState(Unchecked);
  else if (text == "maybe")
    setCheckState(PartiallyChecked);
}

void WAbstractToggleButton::refresh()
{
  if (text_.text.refresh()) {
    textChanged_ = true;
    repaint(RepaintSizeAffected);
  }

  WFormWidget::refresh();
}

}
