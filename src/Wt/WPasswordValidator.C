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

#ifndef WT_DEBUG_JS
#include "js/WPasswordValidator.min.js"
#endif

namespace Wt {

WPasswordValidator::WPasswordValidator()
  : WRegExpValidator(),
    minLength_(0),
    maxLength_(std::numeric_limits<int>::max()),
    tooShortText_(WString::tr("Wt.WPasswordValidator.TooShort")),
    tooLongText_(WString::tr("Wt.WPasswordValidator.TooLong"))
{
  setMandatory(true);
}

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

Wt::WValidator::Result WPasswordValidator::validate(const WT_USTRING& input) const
{
  if (input.empty()) {
    if (isMandatory()) {
      return Result(ValidationState::InvalidEmpty, invalidBlankText());
    } else {
      return Result(ValidationState::Valid);
    }
  }


#ifndef WT_TARGET_JAVA
  std::u32string text = input.toUTF32();
#else
  std::string text = input;
#endif
  int size = static_cast<int>(text.length());

  if (size < minLength_ && (minLength_ <= maxLength_ || maxLength_ < 0)) {
    return Result(ValidationState::Invalid, invalidTooShortText());
  }

  if (size > maxLength_) {
    return Result(ValidationState::Invalid, invalidTooLongText());
  }

  if (regExpPattern().empty()) {
    return Result(ValidationState::Valid);
  }

  return WRegExpValidator::validate(input);

}

std::string WPasswordValidator::javaScriptValidate() const
{
  LOAD_JAVASCRIPT(WApplication::instance(), "js/WPasswordValidator.js", "WPasswordValidator", wtjs1);

  WStringStream js;

  js << "new " WT_CLASS ".WPasswordValidator("
     << isMandatory() << ","
     << minLength() << ","
     << maxLength() << ","
     << WWebWidget::jsStringLiteral(regExpPattern()) << ","
     << WWebWidget::jsStringLiteral(invalidBlankText()) << ","
     << WWebWidget::jsStringLiteral(invalidTooShortText()) << ","
     << WWebWidget::jsStringLiteral(invalidTooLongText()) << ","
     << WWebWidget::jsStringLiteral(invalidNoMatchText())
     << ");";

  return js.str();
}


}