/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractToggleButton.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WTheme.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

LOGGER("WAbstractToggleButton");

const char *WAbstractToggleButton::CHECKED_SIGNAL = "M_checked";
const char *WAbstractToggleButton::UNCHECKED_SIGNAL = "M_unchecked";

WAbstractToggleButton::WAbstractToggleButton()
  : state_(CheckState::Unchecked)
{
  flags_.set(BIT_NAKED);
  flags_.set(BIT_WORD_WRAP);
  text_.format = TextFormat::Plain;
}

WAbstractToggleButton::WAbstractToggleButton(const WString& text)
  : state_(CheckState::Unchecked)
{ 
  flags_.set(BIT_WORD_WRAP);
  text_.format = TextFormat::Plain;
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

  if (isRendered() && flags_.test(BIT_NAKED))
    LOG_ERROR("setText() has no effect when already rendered as a naked "
	      "checkbox (without label)");

  text_.setText(text);
  flags_.reset(BIT_NAKED);
  flags_.set(BIT_TEXT_CHANGED);
  repaint(RepaintFlag::SizeAffected);
}

bool WAbstractToggleButton::setTextFormat(TextFormat format)
{
  return text_.setFormat(format);
}

void WAbstractToggleButton::setChecked(bool how)
{
  setCheckState(how ? CheckState::Checked : CheckState::Unchecked);
}

void WAbstractToggleButton::setCheckState(CheckState state)
{
  if (canOptimizeUpdates() && state == state_)
    return;

  state_ = state;
  flags_.set(BIT_STATE_CHANGED);
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

  DomElement *input = nullptr;
  DomElement *span = nullptr;
  DomElement *label = nullptr;

  // Already apply theme here because it may determine its organization
  if (all)
    app->theme()->apply(this, element, 1);

  if (element.type() == DomElementType::INPUT)
    input = &element;
  else {
    if (all) {
      input = DomElement::createNew(DomElementType::INPUT);
      input->setName("in" + id());

      span = DomElement::createNew(DomElementType::SPAN);
      span->setName("t" + id());
 
      if (element.type() != DomElementType::LABEL) {
	label = DomElement::createNew(DomElementType::LABEL);
	label->setName("l" + id());
      }
    } else {
      input = DomElement::getForUpdate("in" + id(), DomElementType::INPUT);
      span = DomElement::getForUpdate("t" + id(), DomElementType::SPAN);
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
   * kept on the interior element. And also tabindex
   */
  if (&element != input) {
    if (element.properties().find(Property::Class) != 
	element.properties().end())
      input->addPropertyWord(Property::Class, 
			     element.getProperty(Property::Class));
    element.setProperties(input->properties());
    input->clearProperties();

    std::string v = element.getProperty(Wt::Property::Disabled);
    if (!v.empty()) {
      input->setProperty(Wt::Property::Disabled, v);
      element.removeProperty(Wt::Property::Disabled);
    }

    v = element.getProperty(Wt::Property::ReadOnly);
    if (!v.empty()) {
      input->setProperty(Wt::Property::ReadOnly, v);
      element.removeProperty(Wt::Property::ReadOnly);
    }

    v = element.getProperty(Wt::Property::TabIndex);
    if (!v.empty()) {
      input->setProperty(Wt::Property::TabIndex, v);
      element.removeProperty(Wt::Property::TabIndex);
    }

    v = input->getAttribute("title");
    if (!v.empty())
      element.setAttribute("title", v);
  }

  if (flags_.test(BIT_STATE_CHANGED) || all) {
    input->setProperty(Wt::Property::Checked,
		       state_ == CheckState::Unchecked ?
		       "false" : "true");

    if (supportsIndeterminate(env))
      input->setProperty(Wt::Property::Indeterminate,
			 state_ == CheckState::PartiallyChecked ?
			 "true" : "false");
    else
      input->setProperty(Wt::Property::StyleOpacity,
			 state_ == CheckState::PartiallyChecked ?
			 "0.5" : "");

    flags_.reset(BIT_STATE_CHANGED);
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
    if (all || flags_.test(BIT_TEXT_CHANGED)) {
      span->setProperty(Property::InnerHTML, text_.formattedText());
      if (all || flags_.test(BIT_WORD_WRAP_CHANGED)) {
	span->setProperty(Property::StyleWhiteSpace, 
			  flags_.test(BIT_WORD_WRAP) ? "normal" : "nowrap");
	flags_.reset(BIT_WORD_WRAP_CHANGED);
      }
      flags_.reset(BIT_TEXT_CHANGED);
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
  if (!flags_.test(BIT_NAKED))
    return DomElementType::LABEL;
  else
    return DomElementType::INPUT;
}

void WAbstractToggleButton::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_STATE_CHANGED);

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
  if (flags_.test(BIT_STATE_CHANGED) || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values))
    if (formData.values[0] == "i")
      state_ = CheckState::PartiallyChecked;
    else
      state_ = formData.values[0] != "0" ? 
	CheckState::Checked : CheckState::Unchecked;
  else
    if (isEnabled() && isVisible())
      state_ = CheckState::Unchecked;
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
	|| env.agentIsChrome()
	|| (env.agentIsGecko() && 
	    (static_cast<unsigned int>(env.agent()) >=
	     static_cast<unsigned int>(UserAgent::Firefox3_6))));
}

std::string WAbstractToggleButton::formName() const
{
  if (domElementType() == DomElementType::LABEL)
    return "in" + id();
  else
    return WFormWidget::formName();
}

WT_USTRING WAbstractToggleButton::valueText() const
{
  switch (state_) {
  case CheckState::Unchecked: return "no";
  case CheckState::PartiallyChecked: return "maybe";
  default: return "yes";
  }
}

void WAbstractToggleButton::setWordWrap(bool wordWrap)
{
  if (flags_.test(BIT_WORD_WRAP) != wordWrap) {
    flags_.set(BIT_WORD_WRAP, wordWrap);
    flags_.set(BIT_WORD_WRAP_CHANGED);
    repaint(RepaintFlag::SizeAffected);
  }
}

bool WAbstractToggleButton::wordWrap() const
{
  return flags_.test(BIT_WORD_WRAP);
}

void WAbstractToggleButton::setValueText(const WT_USTRING& text)
{
  if (text == "yes")
    setCheckState(CheckState::Checked);
  else if (text == "no")
    setCheckState(CheckState::Unchecked);
  else if (text == "maybe")
    setCheckState(CheckState::PartiallyChecked);
}

void WAbstractToggleButton::refresh()
{
  if (text_.text.refresh()) {
    flags_.set(BIT_TEXT_CHANGED);
    repaint(RepaintFlag::SizeAffected);
  }

  WFormWidget::refresh();
}

}
