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

#include "DomElement.h"
#include "Utils.h"

namespace Wt {

WFormWidget::WFormWidget(WContainerWidget *parent)
  : WInteractWidget(parent),
    changed(this),
    selected(this),
    blurred(this),
    focussed(this),
    label_(0),
    validator_(0),
    validate_(0),
    filterInput_(0)
{
  flags_.set(BIT_ENABLED);

  implementStateless(&WFormWidget::disable, &WFormWidget::undoDisable);
  implementStateless(&WFormWidget::enable, &WFormWidget::undoEnable);
  implementStateless(&WFormWidget::setFocus, &WFormWidget::undoSetFocus);
}

WFormWidget::~WFormWidget()
{
  if (label_)
    label_->setBuddy((WFormWidget *)0);

  if (validator_)
    validator_->removeFormWidget(this);

  delete validate_;
  delete filterInput_;
}

void WFormWidget::enable()
{
  setEnabled(true);
}

void WFormWidget::disable()
{
  setEnabled(false);
}

bool WFormWidget::isEnabled() const
{
  return flags_.test(BIT_ENABLED);
}

void WFormWidget::setFocus()
{
  flags_.set(BIT_GOT_FOCUS);
  repaint(RepaintPropertyIEMobile);
}

void WFormWidget::undoSetFocus()
{
}

void WFormWidget::undoEnable()
{
  setEnabled(flags_.test(BIT_WAS_ENABLED));
}

void WFormWidget::undoDisable()
{
  undoEnable();
}

void WFormWidget::setEnabled(bool enabled)
{
  flags_.set(BIT_WAS_ENABLED, flags_.test(BIT_ENABLED));
  flags_.set(BIT_ENABLED, enabled);
  flags_.set(BIT_ENABLED_CHANGED);

  repaint();
}

void WFormWidget::validatorChanged()
{
  std::string validateJS = validator_->javaScriptValidate(jsRef());
  if (!validateJS.empty()) {
    if (!validate_) {
      validate_ = new JSlot(this);

      keyWentUp.connect(*validate_);
      changed.connect(*validate_);
      clicked.connect(*validate_);
    }

    validate_->setJavaScript
      ("function(self, event){"
       "var v=" + validateJS + ";"
       "self.className= v.valid ? '' : 'Wt-invalid';"
       "if (v.valid) self.removeAttribute('title');"
       "else self.setAttribute('title', v.message);}"); 
  } else {
    delete validate_;
    validate_ = 0;
  }

  std::string inputFilter = validator_->inputFilter();
  if (!inputFilter.empty()) {
    if (!filterInput_) {
      filterInput_ = new JSlot();

      keyPressed.connect(*filterInput_);
    }

    Wt::Utils::replace(inputFilter, '/', "\\/");

    filterInput_->setJavaScript
      ("function(self,e){"
       "var c=String.fromCharCode("
       "(typeof e.charCode!=='undefined')?e.charCode:e.keyCode);"
       "if(/" + inputFilter + "/.test(c)) return true; else{"
       WT_CLASS ".cancelEvent(e);}}");
  } else {
    delete filterInput_;
    filterInput_ = 0;
  }
}

void WFormWidget::updateDom(DomElement& element, bool all)
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (!env.agentIE() || !dynamic_cast<WAbstractToggleButton *>(this))
    updateSignalConnection(element, changed, "change", all);

  updateSignalConnection(element, selected, "select", all);
  updateSignalConnection(element, blurred, "blur", all);
  updateSignalConnection(element, focussed, "focus", all);

  if (flags_.test(BIT_ENABLED_CHANGED) || all) {
    element.setProperty(Wt::PropertyDisabled, isEnabled() ? "false" : "true");
    flags_.reset(BIT_ENABLED_CHANGED);
  }

  if (all && flags_.test(BIT_GOT_FOCUS))
    flags_.set(BIT_INITIAL_FOCUS);
  
  if (flags_.test(BIT_GOT_FOCUS)
      || (all && flags_.test(BIT_INITIAL_FOCUS))) {
    element.callMethod("focus()");
    flags_.reset(BIT_GOT_FOCUS);
  }

  WInteractWidget::updateDom(element, all);
}

void WFormWidget::setLabel(WLabel *label)
{
  if (label_) {
    WLabel *l = label_;
    label_ = 0;
    l->setBuddy((WFormWidget *)0);
  }
  label_ = label;
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
    validator_->addFormWidget(this);
    validatorChanged();
    setStyleClass(validate() == WValidator::Valid ? "" : "Wt-invalid");
  } else {
    delete validate_;
    validate_ = 0;
    delete filterInput_;
    filterInput_ = 0;
  }

  if (!validator_->parent())
    WObject::addChild(validator_);
}

WValidator::State WFormWidget::validate()
{
  return WValidator::Valid;
}

}
