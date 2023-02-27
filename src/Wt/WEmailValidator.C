/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WEmailValidator.h"
#include "Wt/WLocale.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"

#include "web/InfraUtils.h"

#include <boost/algorithm/string.hpp>

#include <regex>

#ifndef WT_DEBUG_JS
#include "js/WEmailValidator.min.js"
#endif

// See: https://html.spec.whatwg.org/multipage/input.html#valid-e-mail-address
#define ATEXT_REGEX "[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+"
#define LET_DIG_REGEX "[a-zA-Z0-9]"
#define LDH_STR_REGEX "[a-zA-Z0-9-]{0,61}"
#define LABEL_REGEX LET_DIG_REGEX "(?:" LDH_STR_REGEX LET_DIG_REGEX ")?"
#define EMAIL_REGEX ATEXT_REGEX "@" LABEL_REGEX "(?:\\." LABEL_REGEX ")*"

namespace Wt {

LOGGER("WEmailValidator");

WEmailValidator::WEmailValidator()
  : multiple_(false)
{ }

void WEmailValidator::setInvalidNotAnEmailAddressText(const WString& text)
{
  notAnEmailAddressText_ = text;
  repaint();
}

WString WEmailValidator::invalidNotAnEmailAddressText() const
{
  if (!notAnEmailAddressText_.empty()) {
    return notAnEmailAddressText_;
  } else if (multiple()) {
    return WString::tr("Wt.WEmailValidator.Invalid.Multiple");
  } else {
    return WString::tr("Wt.WEmailValidator.Invalid");
  }
}

void WEmailValidator::setInvalidNotMatchingText(const WString &text)
{
  notMatchingText_ = text;
  repaint();
}

WString WEmailValidator::invalidNotMatchingText() const
{
  if (!notMatchingText_.empty()) {
    return WString(notMatchingText_).arg(pattern_);
  } else if (multiple()) {
    return WString::tr("Wt.WEmailValidator.NotMatching.Multiple").arg(pattern_);
  } else {
    return WString::tr("Wt.WEmailValidator.NotMatching").arg(pattern_);
  }
}

void WEmailValidator::setMultiple(bool multiple)
{
  if (multiple_ != multiple) {
    multiple_ = multiple;
    repaint();
  }
}

void WEmailValidator::setPattern(const WString& pattern)
{
  if (pattern_ != pattern) {
    pattern_ = pattern;
    repaint();
  }
}

WValidator::Result WEmailValidator::validate(const WT_USTRING& input) const
{
#ifndef WT_TARGET_JAVA
  auto s = input.toUTF8();
#else
  std::string s = input;
#endif

  if (s.empty()) {
    if (isMandatory()) {
      return Result(ValidationState::InvalidEmpty, invalidBlankText());
    } else {
      return Result(ValidationState::Valid);
    }
  }

  bool result = true;
  if (multiple()) {
    std::vector<std::string> splits;
    boost::split(splits, s, boost::is_any_of(","));
    for (const auto& split : splits) {
      if (!validateOne(split)) {
        result = false;
        break;
      }
    }
  } else {
    result = validateOne(s);
  }
  if (result) {
    return Result(ValidationState::Valid);
  } else if (!pattern().empty()) {
    return Result(ValidationState::Invalid, invalidNotMatchingText());
  } else {
    return Result(ValidationState::Invalid, invalidNotAnEmailAddressText());
  }
}

bool WEmailValidator::validateOne(const std::string &emailAddress) const
{
  std::regex emailAddressRegex(EMAIL_REGEX);
  if (!std::regex_match(emailAddress, emailAddressRegex)) {
    return false;
  }
  if (!pattern_.empty() && !std::regex_match(emailAddress, std::regex(pattern_.toUTF8()))) {
    return false;
  }
  return true;
}

std::string WEmailValidator::javaScriptValidate() const
{
  loadJavaScript(WApplication::instance());

  WStringStream js;

  js << "new " WT_CLASS ".WEmailValidator("
     << isMandatory() << ','
     << multiple() << ','
     << (pattern_.empty() ? "null" : WWebWidget::jsStringLiteral(pattern_)) << ','
     << WWebWidget::jsStringLiteral(invalidBlankText()) << ','
     << WWebWidget::jsStringLiteral(invalidNotAnEmailAddressText()) << ','
     << WWebWidget::jsStringLiteral(invalidNotMatchingText())
     << ");";

  return js.str();
}

void WEmailValidator::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/WEmailValidator.js", "WEmailValidator", wtjs1);
}

}
