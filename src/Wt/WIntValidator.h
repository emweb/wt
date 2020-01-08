// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WINTVALIDATOR_H_
#define WINTVALIDATOR_H_

#include <limits>

#include <Wt/WValidator.h>

namespace Wt {

/*! \class WIntValidator Wt/WIntValidator.h Wt/WIntValidator.h
 *  \brief A validator that validates integer user input.
 *
 * This validator checks whether user input is an integer number in a
 * pre-defined range.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WLineEdit *lineEdit = addWidget(std::make_unique<Wt::WLineEdit>());
 * std::shared_ptr<Wt::WIntValidator> validator =
 *   std::make_shared<Wt::WIntValidator>(0, 100);
 * lineEdit->setValidator(validator);
 * lineEdit->setText("50");
 * \endcode
 * \endif
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WIntValidator.NotAnInteger: Must be an integer number
 * - Wt.WIntValidator.TooSmall: The number must be larger than {1}
 * - Wt.WIntValidator.BadRange: The number must be in the range {1} to {2}
 * - Wt.WIntValidator.TooLarge: The number must be smaller than {1}
 */
class WT_API WIntValidator : public WValidator
{
public:
  /*! \brief Creates a new integer validator that accepts any integer.
   *
   * The validator will accept numbers using the current locale's format.
   *
   * \sa WLocale::currentLocale()
   */
  WIntValidator();

  /*! \brief Creates a new integer validator that accepts integer input
   *         within the given range.
   *
   * \sa WLocale::currentLocale()
   */
  WIntValidator(int minimum, int maximum);

  /*! \brief Returns the bottom of the valid integer range.
   */
  int bottom() const { return bottom_; }

  /*! \brief Sets the bottom of the valid integer range.
   *
   * The default value is the minimum integer value.
   */
  void setBottom(int bottom);

  /*! \brief Returns the top of the valid integer range.
   */
  int top() const { return top_; }

  /*! \brief Sets the top of the valid integer range.
   *
   * The default value is the maximum integer value.
   */
  void setTop(int top);

  /*! \brief Sets the range of valid integers.
   */
  virtual void setRange(int bottom, int top);

  /*! \brief Validates the given input.
   *
   * The input is considered valid only when it is blank for a non-mandatory
   * field, or represents an integer within the valid range.
   */
  virtual Result validate(const WT_USTRING& input) const override;

  /*! \brief Sets the message to display when the input is not a number.
   *
   * The default value is "Must be an integer number."
   */
  void setInvalidNotANumberText(const WString& text);

  /*! \brief Returns the message displayed when the input is not a number.
   *
   * \sa setInvalidNotANumberText(const WString&)
   */
  WString invalidNotANumberText() const;

  /*! \brief Sets the message to display when the number is too small
   *
   * Depending on whether bottom() and top() are real bounds, the
   * default message is "The number must be between {1} and {2}" or
   * "The number must be larger than {1}".
   */
  void setInvalidTooSmallText(const WString& text);

  /*! \brief Returns the message displayed when the number is too small.
   *
   * \sa setInvalidTooSmallText(const WString&)
   */
  WString invalidTooSmallText() const;

  /*! \brief Sets the message to display when the number is too large
   *
   * Depending on whether bottom() and top() are real bounds, the
   * default message is "The number must be between {1} and {2}" or
   * "The number must be smaller than {2}".
   */
  void setInvalidTooLargeText(const WString& text);

  /*! \brief Returns the message displayed when the number is too large.
   *
   * \sa setInvalidTooLargeText(const WString&)
   */
  WString invalidTooLargeText() const;
  
  /*! \brief If true the validator will ignore trailing spaces
   *
   * \sa ignoreTrailingSpaces()
   */
  void setIgnoreTrailingSpaces(bool b);

  /*! \brief Indicates whether the validator should ignore the trailing spaces
   * \sa setIgnoreTrailingSpaces()
   */
  bool ignoreTrailingSpaces() { return ignoreTrailingSpaces_; }

  virtual std::string javaScriptValidate() const override;

  virtual std::string inputFilter() const override;

private:
  int bottom_;
  int top_;

  bool ignoreTrailingSpaces_;

  WString tooSmallText_;
  WString tooLargeText_;
  WString nanText_;
  
  static void loadJavaScript(WApplication *app);
};

}

#endif // WINTVALIDATOR_H_
