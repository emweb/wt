
#ifndef WTIME_VALIDATOR_H_
#define WTIME_VALIDATOR_H_

#include "Wt/WRegExpValidator.h"

namespace Wt {

/*! \class WTimeValidator Wt/WTimeValidator.h Wt/WTimeValidator.h
 *  \brief A time validator
 *
 *  \sa WTimeEdit
 *  \sa WTime
 *  \sa WTimePicker
 *
 */
class WT_API WTimeValidator : public WRegExpValidator
{
public:
	/*! \brief Creates a new WTimeValidator
	 */
    WTimeValidator();

	/*! \brief Creates a new WTimeValidator
	 */
    WTimeValidator(const WT_USTRING &format);

    /*! \brief Creates a new WTimeValidator
     *
     * The validator will accept only times within the indicated range
     * <i>bottom</i> to <i>top</i>, in the time formate \p format
     */
    WTimeValidator(const WT_USTRING &format, const WTime &bottom,
                   const WTime &top);

    /*! \brief Sets the validator format
	 * \sa WTime::toString()
	 */
    void setFormat(const WT_USTRING &format);

    /*! \brief Returns the validator current format
	 */
    virtual WT_USTRING format() const override { return formats_[0]; }

    /*! \brief Sets the time formats used to parse time strings
     */
    void setFormats(const std::vector<WT_USTRING> &formats);

    /*! \brief Returns the time formats used to parse time strings
     */
    const std::vector<WT_USTRING>& formats() const { return formats_;}

    /*! \brief Sets the lower limit of the valid time range
     *
     * The default is a null time constructed using WTime()
     */
    void setBottom(const WTime &bottom);

    /*! \brief Returns the lower limit of the valid time range
     */
    const WTime& bottom() const { return bottom_;}

    /*! \brief Sets the upper limit of the valid time range
     *
     * The default is a null time constructed using WTime()
     */
    void setTop(const WTime &top);

    /*! \brief Returns the upper limit of the valid time range
     */
    const WTime& top() const { return top_;}

    /*! \brief Sets the message to display when the input is not a time
     */
    void setInvalidNotATimeText(const WString &text);

    /*! \brief Returns the message displayed when the input is not a time
     */
    WString invalidNotATimeText() const;

    /*! \brief Sets the message to display when the time is earlier than bottom
     */
    void setInvalidTooEarlyText(const WString &text);

    /*! \brief Returns the message displayed when time is too early
     */
    WString invalidTooEarlyText() const;

    /*! \brief Sets the message to display when the time is later than top
     */
    void setInvalidTooLateText(const WString &text);

    /*! \brief Returns the message displayed when time is too late
     */
    WString invalidTooLateText() const;

    /*! \brief Validates the given input
     *
     * The input is considered valid only when it is blank for a
     * non-mandatory field, or represents a time in the given format,
     * and within the valid range.
     */
    virtual Result validate(const WT_USTRING &input) const override;

    virtual std::string javaScriptValidate() const override;

private:

    std::vector<WT_USTRING> formats_;
    WTime bottom_, top_;

    WString tooEarlyText_;
    WString tooLateText_;
    WString notATimeText_;

    static void loadJavaScript(WApplication *app);

};

}

#endif // WTIME_VALIDATOR_H_
