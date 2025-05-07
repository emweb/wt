/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DomElement.h"

#include "Wt/WPasswordEdit.h"
#include "Wt/WPasswordValidator.h"

namespace Wt {

WPasswordEdit::WPasswordEdit()
  : WLineEdit(),
    nativeControl_(false)
{
  init();
}

WPasswordEdit::WPasswordEdit(const WT_USTRING& content)
  : WLineEdit(content),
    nativeControl_(false)
{
  init();
}

void WPasswordEdit::init()
{
  pwdValidator_ = std::make_shared<WPasswordValidator>();
  WLineEdit::setValidator(std::make_shared<WStackedValidator>());
  stackedValidator()->addValidator(pwdValidator_);
}

void WPasswordEdit::setNativeControl(bool nativeControl)
{
  if (nativeControl != nativeControl_) {
    nativeControl_ = nativeControl;

    flags_.set(BIT_CONTROL_CHANGED);

    if (!nativeControl) {
      stackedValidator()->addValidator(pwdValidator_);
    } else {
      stackedValidator()->removeValidator(pwdValidator_);
    }

  }
}

void WPasswordEdit::setMinLength(int length)
{
  if (length != minLength()) {
    pwdValidator_->setMinLength(length);
    flags_.set(BIT_MIN_LENGTH_CHANGED);
    repaint();
  }
}

void WPasswordEdit::setMaxLength(int length)
{
  if (length != maxLength()) {
    pwdValidator_->setMaxLength(length);
  }

  WLineEdit::setMaxLength(length);
}

void WPasswordEdit::setRequired(bool required)
{
  if (required != isRequired()) {
    pwdValidator_->setMandatory(required);
    flags_.set(BIT_REQUIRED_CHANGED);
    repaint();
  }
}

void WPasswordEdit::setPattern(const WT_USTRING& newPattern)
{
  pwdValidator_->setRegExp(newPattern);
  flags_.set(BIT_PATTERN_CHANGED);
  repaint();
}

void WPasswordEdit::setInvalidTooLongText(const WString& text)
{
  pwdValidator_->setInvalidTooLongText(text);
}

void WPasswordEdit::setInvalidTooShortText(const WString& text)
{
  pwdValidator_->setInvalidTooShortText(text);
}

void WPasswordEdit::setInvalidBlankText(const WString& text)
{
  pwdValidator_->setInvalidBlankText(text);
}

void WPasswordEdit::setInvalidNoMatchText(const WString& text)
{
  pwdValidator_->setInvalidNoMatchText(text);
}

void WPasswordEdit::setValidator(const std::shared_ptr<WValidator>& validator)
{
  if (otherValidator_ != validator) {
    if (otherValidator_) {
      stackedValidator()->removeValidator(otherValidator_);
    }

    otherValidator_ = validator;

    if (otherValidator_) {
      stackedValidator()->insertValidator(0, otherValidator_);
    }
  }

  WLineEdit::setValidator(stackedValidator());
}

ValidationState WPasswordEdit::validate()
{
  if (nativeControl_) {
    auto state = pwdValidator_->validate(text()).state();
    if (state != ValidationState::Valid) {
      return state;
    }
  }
  return WLineEdit::validate();
}

void WPasswordEdit::updateDom(DomElement& element, bool all)
{
  bool controlChanged = flags_.test(BIT_CONTROL_CHANGED);

  if (nativeControl_) {

    if (all || controlChanged || flags_.test(BIT_MIN_LENGTH_CHANGED)) {
      if (!all || minLength() > 0) {
        element.setAttribute("minlength", std::to_string(minLength()));
      }
      flags_.reset(BIT_MIN_LENGTH_CHANGED);
    }

    if (all || controlChanged || flags_.test(BIT_REQUIRED_CHANGED)) {
      if (isRequired()) {
        element.setAttribute("required", "");
      } else {
        element.removeAttribute("required");
      }
      flags_.reset(BIT_MIN_LENGTH_CHANGED);
    }

    if (all || controlChanged || flags_.test(BIT_PATTERN_CHANGED)) {
      if (!pattern().empty()) {
        element.setAttribute("pattern", pattern().toUTF8());
      } else {
        element.removeAttribute("pattern");
      }
      flags_.reset(BIT_PATTERN_CHANGED);
    }
  } else if (controlChanged) {
    element.removeAttribute("minlength");
    element.removeAttribute("required");
    element.removeAttribute("pattern");
  }

  WLineEdit::updateDom(element, all);
}

}