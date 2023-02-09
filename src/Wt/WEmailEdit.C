/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WEmailEdit.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WEmailValidator.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WSuggestionPopup.h"
#include "Wt/WValidator.h"

#include "web/DomElement.h"
#include "web/InfraUtils.h"
#include "web/WebUtils.h"

#include <boost/algorithm/string.hpp>

#ifndef WT_DEBUG_JS
#include "js/WEmailEdit.min.js"
#endif

namespace Wt {

LOGGER("WEmailEdit");

const char *WEmailEdit::INPUT_SIGNAL = "input";

WEmailEdit::WEmailEdit()
{
  WWebWidget::setInline(true);
  setFormObject(true);
  setValidator(std::make_shared<WEmailValidator>());
}

#ifndef WT_CNOR
WEmailEdit::~WEmailEdit() = default;
#endif

std::shared_ptr<WEmailValidator> WEmailEdit::emailValidator()
{
  return std::dynamic_pointer_cast<WEmailValidator>(validator());
}

std::shared_ptr<const WEmailValidator> WEmailEdit::emailValidator() const
{
  return std::dynamic_pointer_cast<const WEmailValidator>(validator());
}

void WEmailEdit::setMultiple(bool multiple)
{
  if (multiple != this->multiple()) {
    flags_.set(BIT_MULTIPLE_CHANGED);
    flags_.set(BIT_MULTIPLE, multiple);
    // Sanitization is done differently depending on multiple(), so let's do the sanitization again
    setValueText(valueText());
    auto validator = emailValidator();
    if (validator) {
      validator->setMultiple(multiple);
    }
    repaint();
  }
}

void WEmailEdit::setPattern(const WString &pattern)
{
  if (pattern != this->pattern()) {
    flags_.set(BIT_PATTERN_CHANGED);
    pattern_ = pattern;
    auto validator = emailValidator();
    if (validator) {
      validator->setPattern(pattern_);
    }
    repaint();
  }
}

EventSignal<>& WEmailEdit::textInput()
{
  return *voidEventSignal(INPUT_SIGNAL, true);
}

void WEmailEdit::render(WFlags<RenderFlag> flags)
{
  WFormWidget::render(flags);

  auto app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/WEmailEdit.js", "encodeEmailValue", wtjs1);
  WWebWidget::setJavaScriptMember("wtEncodeValue", WT_CLASS ".encodeEmailValue");
}

void WEmailEdit::updateDom(DomElement& element, bool all)
{
  WFormWidget::updateDom(element, all);

  if (all) {
    element.setAttribute("type", "email");
  }

  if (all || flags_.test(BIT_MULTIPLE_CHANGED)) {
    if (multiple()) {
      element.setAttribute("multiple", "multiple");
    } else if (!all) {
      element.removeAttribute("multiple");
    }
    flags_.reset(BIT_MULTIPLE_CHANGED);
  }

  if (all || flags_.test(BIT_PATTERN_CHANGED)) {
    if (!pattern().empty()) {
      element.setAttribute("pattern", pattern().toUTF8());
    } else if (!all) {
      element.removeAttribute("pattern");
    }
    flags_.reset(BIT_PATTERN_CHANGED);
  }

  if (all || flags_.test(BIT_VALUE_CHANGED)) {
    if (!(all && value_.empty())) {
      element.setProperty(Property::Value, value_.toUTF8());
    }
    flags_.reset(BIT_VALUE_CHANGED);
  }
}

void WEmailEdit::validatorChanged()
{
  const auto validator = emailValidator();
  if (validator) {
    setMultiple(validator->multiple());
    setPattern(validator->pattern());
  }
  // This one goes last so we revalidate the input
  WFormWidget::validatorChanged();
}

WT_USTRING WEmailEdit::sanitize(const WT_USTRING& input,
                                const bool multiple)
{
#ifndef WT_TARGET_JAVA
  auto u8String = input.toUTF8();
#else
  std::string u8String = input;
#endif
  if (multiple) {
    std::vector<std::string> splits;
    boost::split(splits, u8String, boost::is_any_of(","));
    std::size_t resultLength = 0;
    for (auto& split : splits) {
      WHATWG::Infra::trim(split);
      resultLength += split.size() + 1;
    }
    std::string result;
    result.reserve(resultLength > 0 ? resultLength - 1 : 0);
    for (auto& split : splits) {
      if (!result.empty()) {
        result += ',';
      }
      result += split;
    }
    return result;
  } else {
    WHATWG::Infra::stripNewlines(u8String);
    WHATWG::Infra::trim(u8String);
    return u8String;
  }
}

void WEmailEdit::setValueText(const WT_USTRING &value)
{
  value_ = sanitize(value, multiple());
  flags_.set(BIT_VALUE_CHANGED);
  repaint();
}

DomElementType WEmailEdit::domElementType() const
{
  return DomElementType::INPUT;
}

void WEmailEdit::setFormData(const FormData &formData)
{
  // if the value was updated through the API, then ignore the update from
  // the browser, this happens when an action generated multiple events,
  // and we do not want to revert the changes made through the API
  if (flags_.test(BIT_VALUE_CHANGED) || isReadOnly()) {
    return;
  }

  if (!Utils::isEmpty(formData.values)) {
    std::string u8Value = formData.values[0];
#ifndef WT_TARGET_JAVA
    value_ = sanitize(Wt::utf8(u8Value), multiple());
#else
    value_ = sanitize(u8Value, multiple());
#endif
  }
}

}
