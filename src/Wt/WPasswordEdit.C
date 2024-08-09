/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DomElement.h"

#include "Wt/WPasswordEdit.h"

namespace Wt {

WPasswordEdit::WPasswordEdit()
  : WLineEdit()
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

void WPasswordEdit::setMinLength(int length)
{
  if (length != minLength()) {
    pwdValidator_->setMinLength(length);
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

void WPasswordEdit::updateDom(DomElement& element, bool all)
{
  if (all) {
    element.setAttribute("type", "password");
  }

  WLineEdit::updateDom(element, all);
}

}