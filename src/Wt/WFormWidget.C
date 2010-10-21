/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractToggleButton"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WFormWidget"
#include "Wt/WJavaScript"
#include "Wt/WLabel"

#include "JavaScriptLoader.h"
#include "DomElement.h"
#include "Utils.h"

#ifndef WT_DEBUG_JS
#include "js/WFormWidget.min.js"
#endif

namespace Wt {

const char *WFormWidget::CHANGE_SIGNAL = "M_change";
const char *WFormWidget::SELECT_SIGNAL = "select";
const char *WFormWidget::FOCUS_SIGNAL = "focus";
const char *WFormWidget::BLUR_SIGNAL = "blur";

WFormWidget::WFormWidget(WContainerWidget *parent)
  : WInteractWidget(parent),
    label_(0),
    validator_(0),
    validateJs_(0),
    filterInput_(0),
    removeEmptyText_(0),
    tabIndex_(0)
{ }

#ifndef WT_TARGET_JAVA
WStatelessSlot *WFormWidget::getStateless(Method method)
{
  typedef void (WFormWidget::*Type)();

  Type focusMethod = &WFormWidget::setFocus;

  if (method == static_cast<WObject::Method>(focusMethod))
    return implementStateless(&WFormWidget::setFocus,
			      &WFormWidget::undoSetFocus);
  else
    return WInteractWidget::getStateless(method);
}
#endif

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

EventSignal<>& WFormWidget::blurred()
{
  return *voidEventSignal(BLUR_SIGNAL, true);
}

EventSignal<>& WFormWidget::focussed()
{
  return *voidEventSignal(FOCUS_SIGNAL, true);
}

void WFormWidget::setFocus(bool focus)
{
  flags_.set(BIT_GOT_FOCUS, focus);
  repaint(RepaintPropertyIEMobile);

  WApplication *app = WApplication::instance();
  if (focus)
    app->setFocus(id(), -1, -1);
  else if (app->focus() == id())
    app->setFocus(std::string(), -1, -1);
}

void WFormWidget::setFocus()
{
  setFocus(true);
}

void WFormWidget::undoSetFocus()
{
}

bool WFormWidget::hasFocus() const
{
  return WApplication::instance()->focus() == id();
}

void WFormWidget::setEnabled(bool enabled)
{
  setDisabled(!enabled);
}

void WFormWidget::propagateSetEnabled(bool enabled)
{
  flags_.set(BIT_ENABLED_CHANGED);
  repaint(RepaintPropertyAttribute);
  
  WInteractWidget::propagateSetEnabled(enabled);
}

void WFormWidget::setReadOnly(bool readOnly)
{
  flags_.set(BIT_READONLY, readOnly);
  flags_.set(BIT_READONLY_CHANGED);

  repaint(RepaintPropertyAttribute);
}

bool WFormWidget::isReadOnly() const
{
  return flags_.test(BIT_READONLY);
}

void WFormWidget::setEmptyText(const WString& emptyText) 
{
  emptyText_ = emptyText;

  WApplication* app = WApplication::instance();
  const WEnvironment& env = app->environment();

  if (env.ajax()) {
    if (!emptyText_.empty()) {
      const char *THIS_JS = "js/WFormWidget.js";

      if (!app->javaScriptLoaded(THIS_JS)) {
	LOAD_JAVASCRIPT(app, THIS_JS, "WFormWidget", wtjs1);
	app->setJavaScriptLoaded(THIS_JS);
      }

      if (!removeEmptyText_) {
	removeEmptyText_ = new JSlot(this);
      
	focussed().connect(*removeEmptyText_);
	blurred().connect(*removeEmptyText_);
	keyWentDown().connect(*removeEmptyText_);

	std::string jsFunction = 
	  "function(obj, event) {"
	  """jQuery.data(" + jsRef() + ", 'obj').updateEmptyText();"
	  "}";
	removeEmptyText_->setJavaScript(jsFunction);
      }
    } else {
      delete removeEmptyText_;
    }
  } else {
    setToolTip(emptyText);
  }
}

void WFormWidget::render(WFlags<RenderFlag> flags)
{
  if ((flags & RenderFull) && !emptyText_.empty()) {
    WApplication* app = WApplication::instance();
    const WEnvironment& env = app->environment();
    if (env.ajax())
      app->doJavaScript("new " WT_CLASS ".WFormWidget("
			+ app->javaScriptClass() + "," 
			+ jsRef() + ","
			+ "'" + emptyText_.toUTF8() + "');");
  }

  WInteractWidget::render(flags);
}

void WFormWidget::updateEmptyText()
{
  if (!emptyText_.empty() && isRendered())
    WApplication::instance()
      ->doJavaScript("jQuery.data(" + jsRef() + ", 'obj')"
		     ".updateEmptyText();");
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
  std::string validateJS = validator_->javaScriptValidate(jsRef());
  if (!validateJS.empty()) {
    if (!validateJs_) {
      validateJs_ = new JSlot(this);

      keyWentUp().connect(*validateJs_);
      changed().connect(*validateJs_);
      clicked().connect(*validateJs_);
    }

    validateJs_->setJavaScript
      ("function(self, event) {"
       """var v=" + validateJS + ";"
       """if (v.valid) {"
       ""  "self.removeAttribute('title');"
       ""  "$(self).removeClass('Wt-invalid');"
       """} else {"
       ""  "self.setAttribute('title', v.message);"
       ""  "$(self).addClass('Wt-invalid');"
       """}"
       "}");
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
      ("function(self,e){\n"
       """var c=\n"
       ""  "String.fromCharCode((typeof e.charCode!=='undefined') ?"
       ""                       "e.charCode : e.keyCode);\n"
       """if(/" + inputFilter + "/.test(c))\n"
       ""  "return true;\n"
       """else\n"
       ""  WT_CLASS ".cancelEvent(e);\n"
       "}");
  } else {
    delete filterInput_;
    filterInput_ = 0;
  }
}

void WFormWidget::updateDom(DomElement& element, bool all)
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (!env.agentIsIE() || !dynamic_cast<WAbstractToggleButton *>(this)) {
    EventSignal<> *s = voidEventSignal(CHANGE_SIGNAL, false);
    if (s)
      updateSignalConnection(element, *s, "change", all);
  }

  if (flags_.test(BIT_ENABLED_CHANGED) || all) {
    if (!all || !isEnabled())
      element.setProperty(Wt::PropertyDisabled,
			  isEnabled() ? "false" : "true");
    flags_.reset(BIT_ENABLED_CHANGED);
  }

  if (flags_.test(BIT_READONLY_CHANGED) || all) {
    if (!all || isReadOnly())
      element.setProperty(Wt::PropertyReadOnly,
			  isReadOnly() ? "true" : "false");
    flags_.reset(BIT_READONLY_CHANGED);
  }

  if (flags_.test(BIT_TABINDEX_CHANGED) || all) {
    if (!all || tabIndex_)
      element.setProperty(PropertyTabIndex,
			  boost::lexical_cast<std::string>(tabIndex_));
    flags_.reset(BIT_TABINDEX_CHANGED);
  }

  if (isEnabled()) {
    if (all && flags_.test(BIT_GOT_FOCUS))
      flags_.set(BIT_INITIAL_FOCUS);

    if (flags_.test(BIT_GOT_FOCUS)
	|| (all && flags_.test(BIT_INITIAL_FOCUS))) {
      element.callJavaScript("setTimeout(function() {"
			     """var f = " + jsRef() + ";"
			     """if (f) f.focus(); }, "
			     + (env.agentIsIElt(9) ? "500" : "10") + ");");

      flags_.reset(BIT_GOT_FOCUS);
    }
  }

  WInteractWidget::updateDom(element, all);
}

void WFormWidget::propagateRenderOk(bool deep)
{
  flags_.reset(BIT_ENABLED_CHANGED);
  flags_.reset(BIT_TABINDEX_CHANGED);

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

void WFormWidget::setHidden(bool hidden)
{
  if (label_)
    label_->setHidden(hidden);

  WInteractWidget::setHidden(hidden);
}

void WFormWidget::setValidator(WValidator *validator)
{
  if (validator_)
    validator_->removeFormWidget(this);

  validator_ = validator;

  if (validator_) {
#ifndef WT_TARGET_JAVA
    if (!validator_->parent())
      WObject::addChild(validator_);
#endif // WT_TARGET_JAVA

    validator_->addFormWidget(this);
    validatorChanged();
    validate();
#ifndef WT_TARGET_JAVA
    if (!validator_->parent())
      WObject::addChild(validator_);
#endif // WT_TARGET_JAVA
  } else {
    removeStyleClass("Wt-invalid", true);
    delete validateJs_;
    validateJs_ = 0;
    delete filterInput_;
    filterInput_ = 0;
  }

}

WValidator::State WFormWidget::validate()
{
  return WValidator::Valid;
}

std::string WFormWidget::formName() const
{
  return id();
}

void WFormWidget::setTabIndex(int index)
{
  tabIndex_ = index;

  flags_.set(BIT_TABINDEX_CHANGED);

  repaint(RepaintPropertyAttribute);
}

int WFormWidget::tabIndex() const
{
  return tabIndex_;
}

}
