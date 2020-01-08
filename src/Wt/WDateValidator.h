// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WDATE_VALIDATOR_H_
#define WDATE_VALIDATOR_H_

#include <Wt/WDate.h>
#include <Wt/WValidator.h>

namespace Wt {

/*! \class WDateValidator Wt/WDateValidator.h Wt/WDateValidator.h
 *  \brief A validator for date input.
 *
 * This validator accepts input in the given date format, and
 * optionally checks if the date is within a given range.
 *
 * \if cpp
 * The format string used for validating user input are the same as
 * those used by WDate::fromString().
 *
 * Usage example:
 * \code
 * Wt::WLineEdit *lineEdit = addWidget(std::make_unique<Wt::WLineEdit>());
 * auto validator = std::make_shared<Wt::WDateValidator>();
 * validator->setFormat("dd-MM-yyyy");
 * lineEdit->setValidator(validator);
 * lineEdit->setText("01-03-2008");
 * \endcode
 * \endif 
 *
 * \if java
 * The format string used are the ones accepted by java.text.SimpleDateFormat
 * \endif
 *
 * <h3>i18n</h3>
 *
 * The strings used in the WDateValidator can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WDateValidator.DateTooEarly: The date must be after {1}
 * - Wt.WDateValidator.DateTooLate: The date must be before {1}
 * - Wt.WDateValidator.WrongDateRange: The date must be between {1} and {2}
 * - Wt.WDateValidator.WrongFormat: Must be a date in the format '{1}'
 */
class WT_API WDateValidator : public WValidator
{
public:
  /*! \brief Creates a date validator.
   *
   * The validator will accept dates using the current locale's format.
   *
   * \sa WLocale::currentLocale()
   */
  WDateValidator();

  /*! \brief Creates a date validator.
   *
   * The validator will accept dates in the indicated range using the
   * current locale's format.
   *
   * \sa WLocale::currentLocale()
   */
  WDateValidator(const WDate& bottom, const WDate& top);

  /*! \brief Creates a date validator.
   *
   * The validator will accept dates in the date format \p format.
   *
   * \if cpp
   * The syntax for \p format is as in WDate::fromString()
   * \endif
   */
  WDateValidator(const WT_USTRING& format);

  /*! \brief Creates a date validator.
   *
   * The validator will accept only dates within the indicated range
   * <i>bottom</i> to <i>top</i>, in the date format \p format.
   *
   * \if cpp
   * The syntax for \p format is as in WDate::fromString()
   * \endif
   */
  WDateValidator(const WT_USTRING& format,
		 const WDate& bottom,
		 const WDate& top);

  /*! \brief Sets the bottom of the valid date range.
   *
   * The default is a null date constructed using WDate().
   */
  void setBottom(const WDate& bottom);

  /*! \brief Returns the bottom date of the valid range.
   */
  const WDate& bottom() const { return bottom_; }

  /*! \brief Sets the top of the valid date range.
   *
   * The default is a null date constructed using WDate().
   */
  void setTop(const WDate& top);

  /*! \brief Returns the top date of the valid range.
   */
  const WDate& top() const { return top_; }

  /*! \brief Sets the date format used to parse date strings.
   *
   * \sa WDate::fromString()
   */
  void setFormat(const WT_USTRING& format);

  /*! \brief Returns the format string used to parse date strings
   *
   * \sa setFormat()
   */
  virtual WT_USTRING format() const override { return formats_[0]; }

  /*! \brief Sets the date formats used to parse date strings.
   */
  void setFormats(const std::vector<WT_USTRING>& formats);

  /*! \brief Returns the date formats used to parse date strings.
   */
  const std::vector<WT_USTRING>& formats() const { return formats_; }

  /*! \brief Validates the given input.
   *
   * The input is considered valid only when it is blank for a
   * non-mandatory field, or represents a date in the given format,
   * and within the valid range.
   */
  virtual Result validate(const WT_USTRING& input) const override;

  /*! \brief Sets the message to display when the input is not a date.
   *
   * The default message is "The date must be of the format {1}", with
   * as first argument the format string.
   */
  void setInvalidNotADateText(const WString& text);

  /*! \brief Returns the message displayed when the input is not a date.
   *
   * \sa setInvalidNotADateText(const WString&)
   */
  WString invalidNotADateText() const;

  /*! \brief Sets the message to display when the date is earlier than bottom.
   *
   * \if cpp
   *
   * Depending on whether bottom() and top() are
   * defined (see WDate::isNull()), the default message is "The date
   * must be between {1} and {2}" or "The date must be after {1}".
   *
   * \elseif java
   *
   * The default message is "The date must be between {1} and {2}" or
   * "The date must be after {1}".
   *
   * \endif
   */
  void setInvalidTooEarlyText(const WString& text);

  /*! \brief Returns the message displayed when date is too early.
   *
   * \sa setInvalidTooEarlyText(const WString&)
   */
  WString invalidTooEarlyText() const;

  /*! \brief Sets the message to display when the date is later than top.
   *
   * \if cpp
   * Depending on whether bottom() and top() are
   * \link WDate::isNull() defined\endlink, the default message is 
   * "The date must be between {1} and {2}" or "The date must be before {2}".
   * \elseif java
   * Depending on whether bottom() and top() are defined, the default message is
   * "The date must be between {1} and {2}" or "The date must be before {2}".
   * \endif 
   */
  void setInvalidTooLateText(const WString& text);

  /*! \brief Returns the message displayed when the date is too late.
   *
   * \sa setInvalidTooLateText(const WString&)
   */
  WString invalidTooLateText() const;

  virtual std::string javaScriptValidate() const override;

private:
  std::vector<WT_USTRING> formats_;
  WDate   bottom_, top_;

  WString tooEarlyText_;
  WString tooLateText_;
  WString notADateText_;

  static void loadJavaScript(WApplication *app);
};

}

#endif // DATE_VALIDATOR_H_
