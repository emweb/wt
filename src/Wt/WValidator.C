/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Utils.h"

#include "Wt/WFormWidget"
#include "Wt/WValidator"
#include "Wt/WString"

namespace Wt {

WValidator::WValidator(WObject *parent)
  : WObject(parent),
    mandatory_(false)
{ }

WValidator::WValidator(bool mandatory, WObject *parent)
  : WObject(parent),
    mandatory_(mandatory)
{ }

WValidator::~WValidator()
{
  for (int i = formWidgets_.size() - 1; i >= 0; --i)
    formWidgets_[i]->setValidator(0);
}

void WValidator::setMandatory(bool mandatory)
{
  if (mandatory_ != mandatory) {
    mandatory_ = mandatory;
    repaint();
  }
}

void WValidator::setInvalidBlankText(const WString& text)
{
  mandatoryText_ = text;
  repaint();
}

WString WValidator::invalidBlankText() const
{
  if (!mandatoryText_.empty())
    return mandatoryText_;
  else
    return WString::tr("Wt.WValidator.Invalid");
}

void WValidator::fixup(WString& input) const
{ }

WValidator::State WValidator::validate(WT_USTRING& input) const
{
  if (isMandatory()) {
    if (input.empty())
      return InvalidEmpty;
  }

  return Valid;
}

std::string WValidator::javaScriptValidate(const std::string& jsRef) const
{
  if (!mandatory_) {
    return "{valid:true}";
  } else {
    return "function(e,t){"
      "var v=e.value.length!=0;"
      "return {valid:v,message:t};"
      "}(" + jsRef + "," + invalidBlankText().jsStringLiteral() + ")";
  }
}

std::string WValidator::inputFilter() const
{
  return std::string();
}

void WValidator::repaint()
{
  for (unsigned i = 0; i < formWidgets_.size(); ++i)
    formWidgets_[i]->validatorChanged();
}

void WValidator::addFormWidget(WFormWidget *w)
{
  formWidgets_.push_back(w);
}

void WValidator::removeFormWidget(WFormWidget *w)
{
  Utils::erase(formWidgets_, w);
}

#ifndef WT_TARGET_JAVA
void WValidator::createExtConfig(std::ostream& config) const
{
  if (mandatory_) {
    config << ",allowBlank:false";
    if (!mandatoryText_.empty())
      config << ",blankText:" << mandatoryText_.jsStringLiteral();
  }
}
#endif //WT_TARGET_JAVA

}
