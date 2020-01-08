/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractToggleButton.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WFormWidget.h"
#include "Wt/WLabel.h"
#include "Wt/WTheme.h"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WFormWidget.min.js"
#endif

namespace Wt {

const char *WFormWidget::CHANGE_SIGNAL = "M_change";

WFormWidget::WFormWidget()
  : label_(nullptr)
{ }

WFormWidget::~WFormWidget()
{
  if (label_)
    label_->setBuddy(nullptr);

  if (validator_)
    validator_->removeFormWidget(this);
}

EventSignal<>& WFormWidget::changed()
{
  return *voidEventSignal(CHANGE_SIGNAL, true);
}

void WFormWidget::setEnabled(bool enabled)
{
  setDisabled(!enabled);
}

bool WFormWidget::canReceiveFocus() const
{
  return true;
}

int WFormWidget::tabIndex() const
{
  int result = WInteractWidget::tabIndex();

  if (result == std::numeric_limits<int>::min())
    return 0;
  else
    return result;
}

void WFormWidget::propagateSetEnabled(bool enabled)
{
  flags_.set(BIT_ENABLED_CHANGED);
  repaint();
  WInteractWidget::propagateSetEnabled(enabled);
}

void WFormWidget::setReadOnly(bool readOnly)
{
  flags_.set(BIT_READONLY, readOnly);
  flags_.set(BIT_READONLY_CHANGED);

  repaint();
}

bool WFormWidget::isReadOnly() const
{
  return flags_.test(BIT_READONLY);
}

void WFormWidget::setPlaceholderText(const WString& placeholderText)
{
  emptyText_ = placeholderText;

  WApplication* app = WApplication::instance();
  const WEnvironment& env = app->environment();
  if (!env.agentIsIElt(10) &&
    (domElementType() == DomElementType::INPUT || domElementType() == DomElementType::TEXTAREA)) {
    flags_.set(BIT_PLACEHOLDER_CHANGED);
    repaint();
  } else {
    if (env.ajax()) {
      if (!emptyText_.empty()) {
	if (!flags_.test(BIT_JS_OBJECT))
	  defineJavaScript();
	else
	  updateEmptyText();

	if (!removeEmptyText_) {
          removeEmptyText_.reset(new JSlot(this));

	  focussed().connect(*removeEmptyText_);
	  blurred().connect(*removeEmptyText_);
	  keyWentDown().connect(*removeEmptyText_);

	  std::string jsFunction =
	    "function(obj, event) {"
	    """" + jsRef() + ".wtObj.applyEmptyText();"
	    "}";
	  removeEmptyText_->setJavaScript(jsFunction);
	}
      } else {
        removeEmptyText_.reset();
      }
    } else {
      setToolTip(placeholderText);
    }
  }
}

void WFormWidget::defineJavaScript(bool force)
{
  if (force || !flags_.test(BIT_JS_OBJECT)) {
    flags_.set(BIT_JS_OBJECT);

    if (!isRendered())
      return;

    WApplication *app = WApplication::instance();

    LOAD_JAVASCRIPT(app, "js/WFormWidget.js", "WFormWidget", wtjs1);

    setJavaScriptMember(" WFormWidget", "new " WT_CLASS ".WFormWidget("
			+ app->javaScriptClass() + "," 
			+ jsRef() + ","
			+ emptyText_.jsStringLiteral() + ");");
  }
}

void WFormWidget::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full)) {
    if (flags_.test(BIT_JS_OBJECT))
      defineJavaScript(true);

    if (validator()) {
      WValidator::Result result = validator()->validate(valueText());
      WApplication::instance()->theme()
	->applyValidationStyle(this, result, 
			       ValidationStyleFlag::InvalidStyle);
    }
  }

  WInteractWidget::render(flags);
}

void WFormWidget::updateEmptyText()
{
  WApplication *app = WApplication::instance();
  const WEnvironment &env = app->environment();
  if (env.agentIsIElt(10) && isRendered())
    doJavaScript(jsRef() + ".wtObj"
		 ".setEmptyText(" + emptyText_.jsStringLiteral() + ");");
}

void WFormWidget::applyEmptyText()
{
  WApplication *app = WApplication::instance();
  const WEnvironment &env = app->environment();
  if (env.agentIsIElt(10) && isRendered() && !emptyText_.empty())
    doJavaScript(jsRef() + ".wtObj.applyEmptyText();");
}

void WFormWidget::refresh()
{
  if (emptyText_.refresh())
    updateEmptyText();

  WInteractWidget::refresh();
}

void WFormWidget::enableAjax()
{
  if (!emptyText_.empty() && toolTip() == emptyText_) {
    setToolTip("");
    setPlaceholderText(emptyText_);
  }
  
  WInteractWidget::enableAjax();
}

void WFormWidget::validatorChanged()
{
  std::string validateJS = validator_->javaScriptValidate();
  if (!validateJS.empty()) {
    setJavaScriptMember("wtValidate", validateJS);

    if (!validateJs_) {
      validateJs_.reset(new JSlot());
      validateJs_->setJavaScript("function(o){" WT_CLASS ".validate(o)}");

      keyWentUp().connect(*validateJs_);
      changed().connect(*validateJs_);
      if (domElementType() != DomElementType::SELECT)
	clicked().connect(*validateJs_);
    }
  } else
    validateJs_.reset();

  std::string inputFilter = validator_->inputFilter();

  if (!inputFilter.empty()) {
    if (!filterInput_) {
      filterInput_.reset(new JSlot());
      keyPressed().connect(*filterInput_);
    }

    Wt::Utils::replace(inputFilter, '/', "\\/");

    filterInput_->setJavaScript
      ("function(o,e){" WT_CLASS ".filter(o,e,"
       + jsStringLiteral(inputFilter) + ")}");
  } else {
#ifdef WT_TARGET_JAVA
    if (filterInput_) {
      keyPressed().disconnect(*filterInput_);
    }
#endif
    filterInput_.reset();
  }

  validate();
}

void WFormWidget::updateDom(DomElement& element, bool all)
{
  const WEnvironment& env = WApplication::instance()->environment();

  bool onChangeHandledElsewhere = dynamic_cast<WAbstractToggleButton *>(this);

  if (!onChangeHandledElsewhere) {
    EventSignal<> *s = voidEventSignal(CHANGE_SIGNAL, false);
    if (s)
      updateSignalConnection(element, *s, "change", all);
  }

  if (flags_.test(BIT_ENABLED_CHANGED) || all) {
    if (!all || !isEnabled())
      element.setProperty(Wt::Property::Disabled,
			  isEnabled() ? "false" : "true");

    if (!all && isEnabled() && env.agentIsIE()) {
      /*
       * FIXME: implement a workaround for IE, reenabling a checkbox makes
       * the input box loose interactivity.
       */
    }
    flags_.reset(BIT_ENABLED_CHANGED);
  }

  if (flags_.test(BIT_READONLY_CHANGED) || all) {
    if (!all || isReadOnly())
      element.setProperty(Wt::Property::ReadOnly,
			  isReadOnly() ? "true" : "false");
    flags_.reset(BIT_READONLY_CHANGED);
  }

  if (flags_.test(BIT_PLACEHOLDER_CHANGED) || all) {
    if (!all || !emptyText_.empty())
      element.setProperty(Wt::Property::Placeholder, emptyText_.toUTF8());
    flags_.reset(BIT_PLACEHOLDER_CHANGED);
  }

  WInteractWidget::updateDom(element, all);

  if (flags_.test(BIT_VALIDATION_CHANGED)) {
    if (validationToolTip_.empty())
      element.setAttribute("title", toolTip().toUTF8());
    else
      element.setAttribute("title", validationToolTip_.toUTF8());
  }
}

void WFormWidget::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_ENABLED_CHANGED);
  flags_.reset(BIT_VALIDATION_CHANGED);

  WInteractWidget::propagateRenderOk(deep);
}

void WFormWidget::setLabel(WLabel *label)
{
  if (label_) {
    WLabel *l = label_;
    label_ = nullptr;
    l->setBuddy(nullptr);
  }
  label_ = label;

  if (label_)
    label_->setHidden(isHidden());
}

void WFormWidget::setHidden(bool hidden, const WAnimation& animation)
{
  if (label_)
    label_->setHidden(hidden, animation);

  WInteractWidget::setHidden(hidden, animation);
}

void WFormWidget::setToolTip(const WString& text, TextFormat textFormat)
{
  WInteractWidget::setToolTip(text, textFormat);

  if (validator_ && textFormat == TextFormat::Plain) {
    setJavaScriptMember("defaultTT", text.jsStringLiteral());
    validate();
  }
}

void WFormWidget::setValidator(const std::shared_ptr<WValidator>& validator)
{
  bool firstValidator = !validator_;

  if (validator_)
    validator_->removeFormWidget(this);

  validator_ = validator;

  if (validator_) {
    validator_->addFormWidget(this);

    if (firstValidator)
      setToolTip(toolTip());

    validatorChanged();
  } else {
    if (isRendered())
      WApplication::instance()->theme()
	->applyValidationStyle(this, WValidator::Result(), None);

    validateJs_.reset();
    filterInput_.reset();
  }
}

ValidationState WFormWidget::validate()
{
  if (validator()) {
    WValidator::Result result = validator()->validate(valueText());

    if (isRendered())
      WApplication::instance()->theme()
	->applyValidationStyle(this, result, ValidationStyleFlag::InvalidStyle);

    if (validationToolTip_ != result.message()) {
      validationToolTip_ = result.message();
      flags_.set(BIT_VALIDATION_CHANGED);
      repaint();
    }

    validated_.emit(result);

    return result.state();
  } else
    return ValidationState::Valid;
}

std::string WFormWidget::formName() const
{
  return id();
}

}
