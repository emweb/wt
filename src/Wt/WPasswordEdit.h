// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPASSWORDEDIT_H_
#define WPASSWORDEDIT_H_

#include "Wt/WLineEdit.h"
#include "Wt/WPasswordValidator.h"
#include "Wt/WStackedValidator.h"

namespace Wt {

/*! \class WPasswordEdit Wt/WPasswordEdit.h Wt/WPasswordEdit.h
 *  \brief A password edit.
 *
 *  A password edit is a line edit where the character are hidden.
 *
 * The widget corresponds to the HTML <tt>&lt;input type="password"&gt;</tt> tag.
 */
class WT_API WPasswordEdit : public WLineEdit
{
public:
  /*! \brief Creates a password edit with empty content.
   */
  WPasswordEdit();

  /*! \brief Creates a password edit with given content.
   */
  WPasswordEdit(const WT_USTRING& content);

  void setMaxLength(int length) override;

  /*! \brief Changes whether a native HTML5 control is used.
   *
   * When enabled the browser's native attribute for password input
   * (<input type="password">) will be used instead of a validator.
   *
   * This option is set to false by default.
   * \sa nativeControl()
   */
  void setNativeControl(bool nativeControl);

  /*! \brief Returns whether a native HTML5 control is used.
   *
   * \sa setNativeControl(bool)
   */
  bool nativeControl() const { return nativeControl_; }

  /*! \brief Specifies the minimum length of text that can be entered.
   *
   * The default value is 0.
   */
  void setMinLength(int length);

  /*! \brief Returns the minimum length of text that can be entered.
   *
   * \sa setMinLength(int)
   */
  int minLength() const { return pwdValidator_->minLength(); }

  /*! \brief Specifies if the password is required.
   *
   * If true, the password cannot be empty.
   *
   * The default value is true.
   */
  void setRequired(bool required);

  /*! \brief Return if the password is required.
   *
   * \sa setRequired(bool)
   */
  bool isRequired() const { return pwdValidator_->isMandatory(); }

  /*! \brief Specifies the pattern that the password must match
   *
   * The default value is "".
   */
  void setPattern(const WT_USTRING& pattern);

  /*! \brief Return the pattern the password must match.
   *
   * \sa setPattern()
   */
  WString pattern() const { return pwdValidator_->regExpPattern(); }

  /*! \brief Sets the message to display when the password is too long.
   *
   * The default value is "Password too long".
   */
  void setInvalidTooLongText(const WString& text);

  /*! \brief Returns the message displayed when the password is too long.
   *
   * \sa setInvalidTooLongText(const WString&)
   */
  WString invalidTooLongText() const { return pwdValidator_->invalidTooLongText(); }

  /*! \brief Sets the message to display when the password is too small.
   *
   * The default value is "Password too small".
   */
  void setInvalidTooShortText(const WString& text);

  /*! \brief Returns the message displayed when the password is too small.
   *
   * \sa setInvalidTooShortText(const WString&)
   */
  WString invalidTooShortText() const { return pwdValidator_->invalidTooShortText(); }

    /*! \brief Sets the message to display when the password does not match the pattern.
   *
   * The default value is "Invalid input".
   */
  void setInvalidNoMatchText(const WString& text);

  /*! \brief Returns the message displayed when the password does not match the pattern.
   *
   * \sa setInvalidNoMatchText(const WString&)
   */
  WString invalidNoMatchText() const { return pwdValidator_->invalidNoMatchText(); };

  /*! \brief Sets the message to display when the password is empty and required.
   *
   * The default value is "This field cannot be empty".
   */
  void setInvalidBlankText(const WString& text);

  /*! \brief Returns the message displayed when the password is empty and required.
   *
   * \sa setInvalidBlankText(const WString&)
   */
  WString invalidBlankText() const { return pwdValidator_->invalidBlankText(); }

  void setValidator(const std::shared_ptr<WValidator>& validator) override;

  std::shared_ptr<WValidator> validator() const override { return otherValidator_; }

  ValidationState validate() override;

protected:
  void updateDom(DomElement& element, bool all) override;
  WT_NODISCARD std::string type() const noexcept override { return "password"; }
  virtual std::shared_ptr<WValidator> realValidator() const { return WLineEdit::validator(); }

private:
  bool nativeControl_;
  std::shared_ptr<WPasswordValidator> pwdValidator_;
  std::shared_ptr<WValidator> otherValidator_;

  static const int BIT_MIN_LENGTH_CHANGED   = 0;
  static const int BIT_REQUIRED_CHANGED     = 1;
  static const int BIT_CONTROL_CHANGED      = 2;
  static const int BIT_PATTERN_CHANGED      = 3;

  std::bitset<4> flags_;

  std::shared_ptr<WStackedValidator> stackedValidator() const { return std::dynamic_pointer_cast<WStackedValidator>(WLineEdit::validator()); }

  void init();
};

}

#endif // WPASSWORDEDIT_H_