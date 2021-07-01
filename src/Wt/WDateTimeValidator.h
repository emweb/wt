#ifndef WDATETIMEVALIDATOR_H
#define WDATETIMEVALIDATOR_H

#include <Wt/WDateTime.h>
#include <Wt/WValidator.h>

namespace Wt {

/*! \class WDateTimeValidator Wt/WDateTimeValidator.h Wt/WDateTimeValidator.h
 *  \brief A validator for dateTime input.
 *
 * This validator accepts input in the given dateTime format, and
 * optionally checks if the dateTime is within a given range.
 *
 * \if cpp
 * The format string used for validating user input are the same as
 * those used by WDateTime::fromString().
 * \endif
 *
 *  \sa WDateTimeEdit
 *  \sa WDateTime
 */
class WT_API WDateTimeValidator : public WValidator
{
public:
  /*! \brief Creates a datetime validator.
   *
   * The validator will accept datetimes using the current locale's format.
   *
   */
  WDateTimeValidator();

  /*! \brief Creates a datetime validator.
   *
   * The validator will accept only datetimes within the indicated range
   * <i>bottom</i> to <i>top</i>.
   */
  WDateTimeValidator(const WDateTime& bottom, const WDateTime& top);

  /*! \brief Creates a datetime validator.
   *
   * The validator will accept dates in the date format \p format.
   *
   * \if cpp
   * The syntax for \p format is as in WDateTime::fromString()
   * \endif
   */
  WDateTimeValidator(const WT_USTRING& format);

  /*! \brief Creates a datetime validator.
   *
   * The validator will accept only datetimes within the indicated range
   * <i>bottom</i> to <i>top</i>, in the datetime format \p format.
   *
   * \if cpp
   * The syntax for \p format is as in WDateTime::fromString()
   * \endif
   */
  WDateTimeValidator(const WT_USTRING& format,
     const WDateTime& bottom,
     const WDateTime& top);

  /*! \brief Sets the bottom of the valid datetime range.
   *
   * The default is an empty datetime constructed using WDateTime().
   */
  void setBottom(const WDateTime& bottom);

  /*! \brief Returns the bottom datetime of the valid range.
   */
  const WDateTime& bottom() const { return bottom_; }

  /*! \brief Sets the top of the valid datetime range.
   *
   * The default is an empty datetime constructed using WDateTime::WDateTime().
   */
  void setTop(const WDateTime& top);

  /*! \brief Returns the top datetime of the valid range.
   */
  const WDateTime& top() const { return top_; }

  /*! \brief Sets the datetime format used to parse datetime strings.
   *
   * \sa setFormats(), WDateTime::fromString()
   */
  void setFormat(const WT_USTRING& format);

  /*! \brief Returns the format string used to parse datetime strings
   *
   * If multiple formats are specified, the first format in the list is
   * returned.
   *
   * \sa setFormat()
   */
  WT_USTRING format() const override { return formats_[0]; }

  /*! \brief Sets the datetime formats used to parse datetime strings.
   *
   * When multiple formats are given, the first format that correctly
   * parses the input is used.
   *
   * \sa setFormat(), WDateTime::fromString()
   */
  void setFormats(const std::vector<WT_USTRING>& formats);

  /*! \brief Returns the datetime formats used to parse datetime strings.
   */
  const std::vector<WT_USTRING>& formats() const { return formats_; }

  /*! \brief Validates the given input.
   *
   * The input is considered valid only when it is blank for a
   * non-mandatory field, or represents a datetime in the given
   * format, and within the valid range.
   */
  Result validate(const WT_USTRING& input) const override;

  std::string javaScriptValidate() const override;

  /*! \brief Sets the message to display when the input is not a datetime.
   *
   * The default message is "The date and time must match the format {1}",
   * with the format string as the first argument.
   */
  void setInvalidNotADateTimeText(const WString& text);

  /*! \brief Returns the message displayed when the input is not a datetime.
   *
   * \sa setInvalidNotADateTimeText(const WString&)
   */
  WString invalidNotADateTimeText() const;

  /*! \brief Sets the message to display when the datetime
   *   is earlier than bottom().
   *
   * \if cpp
   *
   * Depending on whether bottom() and top() are
   * defined (see WDateTime::isNull()), the default message is
   * "The date and time must be between {1} and {2}" or
   * "The date and time must be after {1}".
   *
   * \elseif java
   *
   * The default message is "The date and time must be between
   * {1} and {2}" or "The date and time must be after {1}".
   *
   * \endif
   */
  void setInvalidTooEarlyText(const WString& text);

  /*! \brief Returns the message displayed when datetime is too early.
   *
   * \sa setInvalidTooEarlyText(const WString&)
   */
  WString invalidTooEarlyText() const;

  /*! \brief Sets the message to display when the datetime is
   *  later than top().
   *
   * \if cpp
   *
   * Depending on whether bottom() and top() are
   * \link WDateTime::isNull() defined\endlink, the default message is
   * "The date and time must be between {1} and {2}" or
   * "The date and time must be before {2}".
   *
   * \elseif java
   *
   * Depending on whether bottom() and top() are defined,
   * the default message is. "The date and time must be between
   * {1} and {2}" or "The date and time must be before {2}".
   *
   * \endif
   */
  void setInvalidTooLateText(const WString& text);

  /*! \brief Returns the message displayed when the datetime is too late.
   *
   * \sa setInvalidTooLateText(const WString&)
   */
  WString invalidTooLateText() const;

private:
    std::vector<WT_USTRING> formats_;
    WDateTime bottom_, top_;

    WString tooEarlyText_;
    WString tooLateText_;
    WString notADateTimeText_;

    static void loadJavaScript(WApplication *app);
};

}
#endif // WDATETIMEVALIDATOR_H
