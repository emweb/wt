// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WPASSWORDEVALIDATOR_H_
#define WPASSWORDEVALIDATOR_H_

#include "Wt/WRegExpValidator.h"

namespace Wt {
/*! \class WPasswordValidator Wt/WPasswordValidator.h Wt/WPasswordValidator.h
 *  \brief A validator that checks if a password is valid
 *
 * This validator validate an input like a browser supporting the attribute
 * maxlength, minlength, required and pattern would.
 */
class WPasswordValidator: public WRegExpValidator
{
public:
  /*! \brief Creates a basic password validator.
   */
  WPasswordValidator();

  /*! \brief Specifies the minimum length of the password.
   *
   * The default value is 0.
   */
  void setMinLength(int chars);

  /*! \brief Returns the minimum length of the password.
   *
   * \sa setMinLength(int)
   */
  int minLength() const { return minLength_; }

  /*! \brief Sets the message to display when the password is too small.
   *
   * The default value is "Password too small".
   */
  void setInvalidTooShortText(const WString& text);

  /*! \brief Returns the message displayed when the password is too small.
   *
   * \sa setInvalidTooShortText(const WString&)
   */
  WString invalidTooShortText() const { return tooShortText_; }

  /*! \brief Specifies the maximum length of the password.
   *
   * The default value is std::numeric_limits<int>::max().
   */
  void setMaxLength(int chars);

  /*! \brief Returns the maximum length of the password.
   *
   * \sa setMaxLength(int)
   */
  int maxLength() const { return maxLength_; }

  /*! \brief Sets the message to display when the password is too long.
   *
   * The default value is "Password too long".
   */
  void setInvalidTooLongText(const WString& text);

  /*! \brief Returns the message displayed when the password is too long.
   *
   * \sa setInvalidTooLongText(const WString&)
   */
  WString invalidTooLongText() const { return tooLongText_; }

  virtual Result validate(const WT_USTRING& input) const override;

  virtual std::string javaScriptValidate() const override;

private:
  int minLength_, maxLength_;
  WString pattern_;
  WString tooShortText_, tooLongText_;

};

}


#endif // WPASSWORDEVALIDATOR_H_