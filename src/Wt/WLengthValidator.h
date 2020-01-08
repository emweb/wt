// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLENGTHVALIDATOR_H_
#define WLENGTHVALIDATOR_H_

#include <limits>

#include <Wt/WValidator.h>

namespace Wt {

/*! \class WLengthValidator Wt/WLengthValidator.h Wt/WLengthValidator.h
 *  \brief A validator that checks the string length of user input.
 *
 * This validator checks whether user input is within the specified range
 * of accepted string lengths.
 *
 * If you only want to limit the length on a line edit, you may also
 * use WLineEdit::setMaxLength().
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WLineEdit *lineEdit = addWidget(std::make_unique<Wt::WLineEdit>());
 * auto validator = std::make_shared<Wt::WLengthValidator>(5, 15);
 * lineEdit->setValidator(validator);
 * lineEdit->setText("abcdef");
 * \endcode
 * \endif
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WLengthValidator.TooShort: The input must be at least {1} characters
 * - Wt.WLengthValidator.BadRange: The input must have a length between {1} and {2} characters
 * - Wt.WLengthValidator.TooLong: The input must be no more than {1} characters
 */
class WT_API WLengthValidator : public WValidator
{
public:
  /*! \brief Creates a length validator that accepts input of any length.
   */
  WLengthValidator();

  /*! \brief Creates a length validator that accepts input within a length
   *         range.
   */
  WLengthValidator(int minLength, int maxLength);

  /*! \brief Sets the minimum length.
   *
   * The default value is 0.
   */
  void setMinimumLength(int minimum);

  /*! \brief Returns the minimum length.
   *
   * \sa setMinimumLength(int)
   */
  int minimumLength() const { return minLength_; }

  /*! \brief Sets the maximum length.
   *
   * The default value is the maximum integer value. 
   */
  void setMaximumLength(int maximum);

  /*! \brief Returns the maximum length.
   *
   * \sa setMaximumLength(int)
   */
  int maximumLength() const { return maxLength_; }

  /*! \brief Validates the given input.
   *
   * The input is considered valid only when it is blank for a non-mandatory
   * field, or has a length within the valid range.
   */
  virtual Result validate(const WT_USTRING& input) const override;

  /*! \brief Sets the message to display when the input is too short.
   *
   * Depending on whether maximumLength() is a real bound, the default
   * message is "The input must have a length between {1} and {2}
   * characters" or " "The input must be at least {1} characters".
   */
  void setInvalidTooShortText(const WString& text);

  /*! \brief Returns the message displayed when the input is too short.
   *
   * \sa setInvalidTooShortText(const WString&)
   */
  WString invalidTooShortText() const;

  /*! \brief Sets the message to display when the input is too long.
   *
   * Depending on whether minimumLength() is different from zero, the
   * default message is "The input must have a length between {1} and
   * {2} characters" or " "The input must be no more than {2}
   * characters".
   */
  void setInvalidTooLongText(const WString& text);

  /*! \brief Returns the message displayed when the input is too long.
   *
   * \sa setInvalidTooLongText(const WString&)
   */
  WString invalidTooLongText() const;

  virtual std::string javaScriptValidate() const override;

private:
  int minLength_;
  int maxLength_;

  WString tooLongText_;
  WString tooShortText_;

  static void loadJavaScript(WApplication *app);
};

}

#endif // WLENGTHVALIDATOR_H_
