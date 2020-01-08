// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTIME_EDIT_H_
#define WTIME_EDIT_H_

#include <Wt/WLineEdit.h>
#include <Wt/WTime.h>
#include <Wt/WTimeValidator.h>
#include <Wt/WTimePicker.h>

namespace Wt {

/*! \class WTimeEdit Wt/WTimeEdit.h Wt/WTimeEdit.h
 *  \brief A Time field editor
 *
 *  \sa WTime
 *  \sa WTimeValidator
 *
 * Styling through CSS is not applicable.
 */
class WT_API WTimeEdit : public WLineEdit
{
public:
    /*! \brief Creates a new time edit.
     */
    WTimeEdit();

    virtual ~WTimeEdit();

    /*! \brief Sets the time
     *
     *  Does nothing if the current time is \p Null.
     *
     * \sa time()
     */
    void setTime(const WTime& time);

    /*! \brief Returns the time.
     *
     * Returns an invalid time (for which WTime::isValid() returns
     * \c false) if the time could not be parsed using the current
     * format().
     *
     * \sa setTime(), WTime::fromString(), WLineEdit::text()
     */
    WTime time() const;

    /*! \brief Returns the validator
     *
     * \sa WTimeValidator
     */
    virtual std::shared_ptr<WTimeValidator> timeValidator() const;

    /*! \brief Sets the format of the Time
     */
    void setFormat(const WT_USTRING& format);

    /*! \brief Returns the format.
     */
    WT_USTRING format() const;

    virtual void setHidden(bool hidden,
			   const WAnimation& animation = WAnimation()) override;

    /*! \brief Sets the lower limit of the valid time range
     */
    void setBottom(const WTime &bottom);

    /*! \brief Returns the lower limit of the valid time range
     */
    WTime bottom() const;

    /*! \brief Sets the upper limit of the valid time range
     */
    void setTop(const WTime &top);

    /*! \brief Returns the upper limit of the valid time range
     */
    WTime top() const;

    /*! \brief Sets the step size for the hours
     */
    void setHourStep(int step);

    /*! \brief Returns the step size for the hours
     */
    int hourStep() const;

    /*! \brief Sets the step size for the minutes
     */
    void setMinuteStep(int step);

    /*! \brief Returns the step size for the minutes
     */
    int minuteStep() const;

    /*! \brief Sets the step size for the seconds
     */
    void setSecondStep(int step);

    /*! \brief Returns the step size for the seconds
     */
    int secondStep() const;

    /*! \brief Sets the step size for the milliseconds
     */
    void setMillisecondStep(int step);

    /*! \brief Returns the step size for the milliseconds
     */
    int millisecondStep() const;

    /*! \brief Enables or disables wraparound
     *
     * Wraparound is enabled by default
     */
    void setWrapAroundEnabled(bool rollover);

    /*! \brief Returns whether wraparound is enabled
     */
    bool wrapAroundEnabled() const;

    virtual void load() override;

protected:
    virtual void render(WFlags<RenderFlag> flags) override;
    virtual void propagateSetEnabled(bool enabled) override;

private:
    std::unique_ptr<WPopupWidget> popup_;
    WTimePicker *timePicker_;

    void setFromTimePicker();
    void setFromLineEdit();
    void defineJavaScript();
    void connectJavaScript(Wt::EventSignalBase& s,
			   const std::string& methodName);
};

}

#endif // WTIME_EDIT_H_
