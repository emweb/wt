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
  setEchoMode(EchoMode::Password);
  pwdValidator_ = std::make_shared<WPasswordValidator>();
  setValidator(pwdValidator_);
}

WPasswordEdit::WPasswordEdit(const WT_USTRING& content)
  : WPasswordEdit()
{
  setText(content);
}

void WPasswordEdit::setNativeControl(bool nativeControl)
{
  if (nativeControl != nativeControl_) {
    nativeControl_ = nativeControl;

    flags_.set(BIT_CONTROL_CHANGED);

    if (!nativeControl) {
      setValidator(pwdValidator_);
    } else {
      setValidator(nullptr);
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
  if (all) {
    element.setAttribute("type", "password");
  }

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
  } else if (controlChanged) {
    element.removeAttribute("minlength");
    element.removeAttribute("required");
  }

  WLineEdit::updateDom(element, all);
}

}