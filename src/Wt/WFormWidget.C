/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractToggleButton"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WFormWidget"
#include "Wt/WLabel"
#include "Wt/WTheme"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WFormWidget.min.js"
#endif

namespace Wt {

const char *WFormWidget::CHANGE_SIGNAL = "M_change";
const char *WFormWidget::SELECT_SIGNAL = "select";

WFormWidget::WFormWidget(WContainerWidget *parent)
  : WInteractWidget(parent),
    label_(0),
    validator_(0),
    validateJs_(0),
    filterInput_(0),
    removeEmptyText_(0)
{ }

WFormWidget::~WFormWidget()
{
  if (label_)
    label_->setBuddy((WFormWidget *)0);

  if (validator_)
    validator_->removeFormWidget(this);

  delete removeEmptyText_;
  delete validateJs_;
  delete filterInput_;
}

EventSignal<>& WFormWidget::changed()
{
  return *voidEventSignal(CHANGE_SIGNAL, true);
}

EventSignal<>& WFormWidget::selected()
{
  return *voidEventSignal(SELECT_SIGNAL, true);
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

void WFormWidget::setEmptyText(const WString& emptyText) 
{
  setPlaceholderText(emptyText);
}

void WFormWidget::setPlaceholderText(const WString& placeholderText)
{
  emptyText_ = placeholderText;

  WApplication* app = WApplication::instance();
  const WEnvironment& env = app->environment();
  if (!env.agentIsIElt(10) &&
    (domElementType() == DomElement_INPUT || domElementType() == DomElement_TEXTAREA)) {
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
	  removeEmptyText_ = new JSlot(this);

	  focussed().connect(*removeEmptyText_);
	  blurred().connect(*removeEmptyText_);
	  keyWentDown().connect(*removeEmptyText_);

	  std::string jsFunction =
	    "function(obj, event) {"
	    """jQuery.data(" + jsRef() + ", 'obj').applyEmptyText();"
	    "}";
	  removeEmptyText_->setJavaScript(jsFunction);
	}
      } else {
	delete removeEmptyText_;
	removeEmptyText_ = 0;
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
  if ((flags & RenderFull)) {
    if (flags_.test(BIT_JS_OBJECT))
      defineJavaScript(true);

    if (validator()) {
      WValidator::Result result = validator()->validate(valueText());
      WApplication::instance()->theme()
	->applyValidationStyle(this, result, ValidationInvalidStyle);
    }
  }

  WInteractWidget::render(flags);
}

void WFormWidget::updateEmptyText()
{
  WApplication *app = WApplication::instance();
  const WEnvironment &env = app->environment();
  if (env.agentIsIElt(10) && isRendered())
    doJavaScript("jQuery.data(" + jsRef() + ", 'obj')"
		 ".setEmptyText(" + emptyText_.jsStringLiteral() + ");");
}

void WFormWidget::applyEmptyText()
{
  WApplication *app = WApplication::instance();
  const WEnvironment &env = app->environment();
  if (env.agentIsIElt(10) && isRendered() && !emptyText_.empty())
    doJavaScript("jQuery.data(" + jsRef() + ", 'obj').applyEmptyText();");
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
    setEmptyText(emptyText_);
  }
  
  WInteractWidget::enableAjax();
}

void WFormWidget::validatorChanged()
{
  std::string validateJS = validator_->javaScriptValidate();
  if (!validateJS.empty()) {
    setJavaScriptMember("wtValidate", validateJS);

    if (!validateJs_) {
      validateJs_ = new JSlot();
      validateJs_->setJavaScript("function(o){" WT_CLASS ".validate(o)}");

      keyWentUp().connect(*validateJs_);
      changed().connect(*validateJs_);
      if (domElementType() != DomElement_SELECT)
	clicked().connect(*validateJs_);
    }
  } else {
    delete validateJs_;
    validateJs_ = 0;
  }

  std::string inputFilter = validator_->inputFilter();

  if (!inputFilter.empty()) {
    if (!filterInput_) {
      filterInput_ = new JSlot();

      keyPressed().connect(*filterInput_);
    }

    Wt::Utils::replace(inputFilter, '/', "\\/");

    filterInput_->setJavaScript
      ("function(o,e){" WT_CLASS ".filter(o,e,"
       + jsStringLiteral(inputFilter) + ")}");
  } else {
#ifndef WT_TARGET_JAVA
    delete filterInput_;
    filterInput_ = 0;
#else 
    if(filterInput_) {
      keyPressed().disconnect(*filterInput_);
      filterInput_ = 0;
    }
#endif
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
      element.setProperty(Wt::PropertyDisabled,
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
      element.setProperty(Wt::PropertyReadOnly,
			  isReadOnly() ? "true" : "false");
    flags_.reset(BIT_READONLY_CHANGED);
  }

  if (flags_.test(BIT_PLACEHOLDER_CHANGED) || all) {
    if (!all || !emptyText_.empty())
      element.setProperty(Wt::PropertyPlaceholder, emptyText_.toUTF8());
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
    label_ = 0;
    l->setBuddy((WFormWidget *)0);
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

  if (validator_ && textFormat == PlainText) {
    setJavaScriptMember("defaultTT", text.jsStringLiteral());
    validate();
  }
}

void WFormWidget::setValidator(WValidator *validator)
{
  bool firstValidator = !validator_;

  if (validator_)
    validator_->removeFormWidget(this);

  validator_ = validator;

  if (validator_) {
#ifndef WT_TARGET_JAVA
    if (!validator_->parent())
      WObject::addChild(validator_);
#endif // WT_TARGET_JAVA

    validator_->addFormWidget(this);

    if (firstValidator)
      setToolTip(toolTip());

    validatorChanged();
#ifndef WT_TARGET_JAVA
    if (!validator_->parent())
      WObject::addChild(validator_);
#endif // WT_TARGET_JAVA
  } else {
    if (isRendered())
      WApplication::instance()->theme()
	->applyValidationStyle(this, WValidator::Result(), ValidationNoStyle);

    delete validateJs_;
    validateJs_ = 0;
    delete filterInput_;
    filterInput_ = 0;
  }
}

WValidator::State WFormWidget::validate()
{
  if (validator()) {
    WValidator::Result result = validator()->validate(valueText());

    if (isRendered())
      WApplication::instance()->theme()
	->applyValidationStyle(this, result, ValidationInvalidStyle);

    if (validationToolTip_ != result.message()) {
      validationToolTip_ = result.message();
      flags_.set(BIT_VALIDATION_CHANGED);
      repaint();
    }

    validated_.emit(result);

    return result.state();
  } else
    return WValidator::Valid;
}

std::string WFormWidget::formName() const
{
  return id();
}

}
