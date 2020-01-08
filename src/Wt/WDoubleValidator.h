// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDOUBLEVALIDATOR_H_
#define WDOUBLEVALIDATOR_H_

#include <limits>

#include <Wt/WValidator.h>

namespace Wt {

/*! \class WDoubleValidator Wt/WDoubleValidator.h Wt/WDoubleValidator.h
 *  \brief A validator for validating floating point user input.
 *
 * This validator checks whether user input is a double in the pre-defined
 * range.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WLineEdit *lineEdit = addWidget(std::make_unique<Wt::WLineEdit>());
 * auto validator = std::make_shared<Wt::WDoubleValidator>();
 * lineEdit->setValidator(validator);
 * lineEdit->setText("13.42");
 * \endcode
 * \endif
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * Wt.WDoubleValidator.NotANumber: Must be a number
 * Wt.WDoubleValidator.TooSmall: The number must be larger than {1}
 * Wt.WDoubleValidator.BadRange: The number must be in the range {1} to {2}
 * Wt.WDoubleValidator.TooLarge: The number must be smaller than {1}
 */
class WT_API WDoubleValidator : public WValidator
{
public:
  /*! \brief Creates a new double validator that accepts any double.
   *
   * The validator will accept numbers using the current locale's format.
   *
   * \sa WLocale::currentLocale()
   */
  WDoubleValidator();

  /*! \brief Creates a new double validator that accepts double
   *         within the given range.
   *
   * The validator will accept numbers using the current locale's format.
   *
   * \sa WLocale::currentLocale()
   */
  WDoubleValidator(double minimum, double maximum);

  /*! \brief Returns the bottom of the valid double range.
   */
  double bottom() const { return bottom_; }

  /*! \brief Sets the bottom of the valid double range.
   *
   * The default value is the minimum double value.
   */
  void setBottom(double bottom);

  /*! \brief Returns the top of the valid double range.
   */
  double top() const { return top_; }

  /*! \brief Sets the top of the valid double range.
   *
   * The default value is the maximum double value.
   */
  void setTop(double top);

  /*! \brief Sets the range of valid doubles.
   */
  virtual void setRange(double bottom, double top);

  /*! \brief Validates the given input.
   *
   * The input is considered valid only when it is blank for a non-mandatory
   * field, or represents a double within the valid range.
   */
  virtual Result validate(const WT_USTRING& input) const override;

  /*! \brief Sets the message to display when the input is not a number.
   *
   * The default value is "Must be a number."
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

private:
  double bottom_;
  double top_;

  bool ignoreTrailingSpaces_;

  WString tooSmallText_;
  WString tooLargeText_;
  WString nanText_;

  static void loadJavaScript(WApplication *app);
};

}

#endif // WDOUBLEVALIDATOR_H_
