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
 * Styling through CSS is not applicable.
 * A native HTML5 control can be used by means of setNativeControl().
 *
 * \note Using the native HTML5 control forces the format \c HH:mm or
 * \c HH:mm:ss. The user will not be aware of this, since the control
 * offers them a view dependent on their locale.
 *
 * \sa WTime
 * \sa WTimeValidator
 * \sa setNativeControl
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

    /*! \brief Sets the format of the Time
     *
     * This sets the format in the validator.
     *
     * When the native HTML5 control is used, the format is limited to
     * \c HH:mm:ss or \c HH:mm. Changing to another format has no effect.
     *
     * \sa WTimeValidator::setFormat()
     * \sa setNativeControl()
     */
    void setFormat(const WT_USTRING& format);

    //! \brief Returns the format.
    WT_USTRING format() const;

    virtual void setHidden(bool hidden,
                           const WAnimation& animation = WAnimation()) override;

    /*! \brief Changes whether the native HTML5 control is used.
     *
     * When enabled the browser's native time input
     * (<tt><input type="time"></tt>) will be used if available.
     * This should provide a better experience on mobile browsers.
     * This option is set to false by default.
     *
     * Calling native control after the widget is rendered is
     * not supported.
     *
     * Setting native control to true requires both \c HH:mm:ss and
     * \c HH:mm to be valid formats. The format can be set by either the
     * validator, or directly with setFormat().
     *
     * Once the format is set, the step will be automatically
     * calculated. This indicates the minimum increment in seconds or
     * minutes that is valid input.
     *
     * When setting native control to true the setters for the steps
     * will no longer do anything.
     *
     * \sa nativeControl()
     * \sa setFormat()
     * \sa setHourStep()
     * \sa setMinuteStep()
     * \sa setSecondStep()
     * \sa setMillisecondStep()
     * \sa setWrapAroundEnabled()
     */
    void setNativeControl(bool nativeControl);

    /*! \brief Returns whether a native HTML5 control is used.
     *
     * When active, the format of the input it limited to \c HH:mm or
     * \c HH:mm:ss. The step is set to \c 60, or \c 1 respectively,
     * specifying the granularity of the input to a minute or a second.
     *
     * \sa setNativeControl()
     */
    WT_NODISCARD bool nativeControl() const noexcept { return nativeControl_; }

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
     *
     * It has no effect if a native HTML5 control is used.
     *
     * \sa setNativeControl()
     */
    void setHourStep(int step);

    /*! \brief Returns the step size for the hours
     */
    int hourStep() const;

    /*! \brief Sets the step size for the minutes
     *
     * It has no effect if a native HTML5 control is used.
     *
     * \sa setNativeControl()
     */
    void setMinuteStep(int step);

    /*! \brief Returns the step size for the minutes
     */
    int minuteStep() const;

    /*! \brief Sets the step size for the seconds
     *
     * It has no effect if a native HTML5 control is used.
     *
     * \sa setNativeControl()
     */
    void setSecondStep(int step);

    /*! \brief Returns the step size for the seconds
     */
    int secondStep() const;

    /*! \brief Sets the step size for the milliseconds
     *
     * It has no effect if a native HTML5 control is used.
     *
     * \sa setNativeControl()
     */
    void setMillisecondStep(int step);

    /*! \brief Returns the step size for the milliseconds
     */
    int millisecondStep() const;

    /*! \brief Enables or disables wraparound
     *
     * It has no effect if a native HTML5 control is used.
     *
     * Wraparound is enabled by default
     */
    void setWrapAroundEnabled(bool rollover);

    /*! \brief Returns whether wraparound is enabled
     */
    bool wrapAroundEnabled() const;

    virtual void load() override;

    /*! \brief Returns the validator
     *
     * \note Using the validator to change the format while
     * a native control is being used will break the
     * native control. If a native control is used, do not call
     * WTimeValidator()::setFormat(), instead use
     * WTimeEdit::setFormat().
     *
     * \sa WTimeValidator::WTimeValidator()
     * \sa setFormat()
     */
    virtual std::shared_ptr<WTimeValidator> timeValidator() const;

protected:
  void render(WFlags<RenderFlag> flags) override;
  void propagateSetEnabled(bool enabled) override;
  void updateDom(DomElement& element, bool all) override;
  void validatorChanged() override;
  WT_NODISCARD std::string type() const noexcept override;

private:
  std::unique_ptr<WPopupWidget> popup_;
  std::unique_ptr<WTimePicker> uTimePicker_;
  Wt::Core::observing_ptr<WTimePicker> oTimePicker_;
  bool nativeControl_;

  void init();
  void setFromTimePicker();
  void setFromLineEdit();
  void defineJavaScript();
  void connectJavaScript(Wt::EventSignalBase& s,
                         const std::string& methodName);
  WT_NODISCARD const char* step() const noexcept;
};

}

#endif // WTIME_EDIT_H_
