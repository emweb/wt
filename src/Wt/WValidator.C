/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WebUtils.h"

#include "Wt/WFormWidget.h"
#include "Wt/WValidator.h"
#include "Wt/WString.h"

namespace Wt {

WValidator::Result::Result()
  : state_(ValidationState::Invalid)
{ }

WValidator::Result::Result(ValidationState state, const WString& message)
  : state_(state),
    message_(message)
{ }

WValidator::Result::Result(ValidationState state)
  : state_(state),
    message_(WString::Empty)
{ }

WValidator::WValidator(bool mandatory)
  : mandatory_(mandatory)
{ }

WValidator::~WValidator()
{
  for (int i = formWidgets_.size() - 1; i >= 0; --i)
    formWidgets_[i]->setValidator(nullptr);
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

WValidator::Result WValidator::validate(const WT_USTRING& input) const
{
  if (isMandatory()) {
    if (input.empty())
      return Result(ValidationState::InvalidEmpty, invalidBlankText());
  }

  return Result(ValidationState::Valid);  
}

WT_USTRING WValidator::format() const
{
  return WT_USTRING();
}

std::string WValidator::javaScriptValidate() const
{
  if (mandatory_) {
    return "new (function() {"
      "this.validate = function(text) {"
      """return { valid: text.length != 0, "
      """message: " + invalidBlankText().jsStringLiteral() + "}"
      "};"
      "})();";
  } else {
    return "new (function() {"
      "this.validate = function(text) {"
      """return { valid: true }"
      "};"
      "})();";
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

}
