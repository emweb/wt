/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WPasswordValidator.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"

#include <limits>

namespace Wt {

WPasswordValidator::WPasswordValidator()
  : WValidator(true),
    minLength_(0),
    maxLength_(std::numeric_limits<int>::max()),
    tooShortText_(WString::tr("Wt.WPasswordValidator.TooShort")),
    tooLongText_(WString::tr("Wt.WPasswordValidator.TooLong"))
{ }

void WPasswordValidator::setMinLength(int chars)
{
  minLength_ = chars;
  repaint();
}

void WPasswordValidator::setMaxLength(int chars)
{
  maxLength_ = chars;
  repaint();
}

void WPasswordValidator::setInvalidTooShortText(const WString& text)
{
  tooShortText_ = text;
  repaint();
}

void WPasswordValidator::setInvalidTooLongText(const WString& text)
{
  tooLongText_ = text;
  repaint();
}

Wt::WPasswordValidator::Result WPasswordValidator::validate(const WT_USTRING& input) const
{
  if (!isMandatory() && input.empty()) {
      return Result(ValidationState::Valid);
  }

  if (isMandatory() && input.empty()) {
      return Result(ValidationState::InvalidEmpty, invalidBlankText());
  }

  int size = input.value().size();

  if (size < minLength_ && (minLength_ <= maxLength_ || maxLength_ < 0)) {
    return Result(ValidationState::Invalid, invalidTooShortText());
  }

  if (size > maxLength_) {
    return Result(ValidationState::Invalid, invalidTooLongText());
  }

  return Result(ValidationState::Valid);

}

std::string WPasswordValidator::javaScriptValidate() const
{
  LOAD_JAVASCRIPT(WApplication::instance(), "js/WPasswordValidator.js", "WPasswordValidator", wtjs1);

  WStringStream js;

  js << "new " WT_CLASS ".WPasswordValidator("
     << isMandatory() << ","
     << minLength() << ","
     << maxLength() << ","
     << invalidBlankText().jsStringLiteral() << ","
     << invalidTooShortText().jsStringLiteral() << ","
     << invalidTooLongText().jsStringLiteral() << ");";

  return js.str();
}


}