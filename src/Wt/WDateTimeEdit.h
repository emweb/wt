// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WDATETIME_EDIT_H_
#define WDATETIME_EDIT_H_

#include <Wt/WCalendar.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPopupWidget.h>
#include <Wt/WTimePicker.h>

namespace Wt {
  class WDateTime;
  class WDateTimeValidator;
/*! \class WDateTimeEdit Wt/WDateTimeEdit.h Wt/WDateTimeEdit Wt/WDateTimeEdit.h
 *  \brief A Date and Time field editor
 *
 *  This editor is a line edit with support for a datetime entry. Users can
 *  fill in the datetime manually or rely on the popup. Using the icon of a
 *  calendar and clock.
 *
 *  In the popup a WTimePicker and WCalendar are shown to the user, allowing
 *  input in a more structured manner.
 *
 *  \note The WTimePicker does not support timezones, and consequentially
 *  this widget doesn't either. If supplied, it is ignored.
 *
 *  \sa WDateTime
 *  \sa WDateTimeValidator
 *  \sa WCalendar
 */
class WT_API WDateTimeEdit : public WLineEdit
{
public:
  /*! \brief Creates a new DateTimeEdit.
   */
  WDateTimeEdit();

  virtual ~WDateTimeEdit();

  /*! \brief Sets the date and time.
   *
   * Does nothing if the \p dateTime is \c null.
   *
   * \sa dateTime
   */
  void setDateTime(const WDateTime& dateTime);

  /*! \brief Returns the datetime.
   *
   * Reads the current datetime.
   *
   * \if cpp
   * Returns an invalid datetime (for which WDateTime::isValid() returns
   * \c false) if the datetime could not be parsed using the current
   * format().
   * \elseif java
   * Returns \c null if the datetime could not be parsed using the current
   * format().
   * \endif
   *
   * \sa setDateTime(), WDateTime::fromString(), WLineEdit::text()
   */
  WDateTime dateTime() const;

  /*! \brief Returns the datetime validator.
   *
   * Most of the configuration of the date time edit is stored in the
   * validator.
   *
   * \sa WDateTimeValidator::WDateTimeValidator()
   */
  virtual std::shared_ptr<WDateTimeValidator> dateTimeValidator() const;

  /*! \brief Configures whether a native HTML5 control should be used.
   *
   * When enabled, the browser's native datetime input
   * (<input type="datetime-local">) will be used if available.
   * This should provide a better experience on mobile browsers.
   *
   * Setting native control to true limits the format to "yyyy-MM-ddThh:mm".
   *
   * There is no support for changing the native control after
   * the widget is rendered.
   *
   * By default, this is set to the value of defaultNativeControl().
   *
   * \sa nativeControl(), setFormat()
   */
  void setNativeControl(bool nativeControl);

  /*! \brief Returns whether a native HTML5 control is used, if supported.
   *
   * Taking into account the preference for a native control,
   * configured using setNativeControl(), this method returns whether
   * a native control is actually being used.
   *
   * \sa setNativeControl()
   */
  WT_NODISCARD bool nativeControl() const noexcept { return nativeControl_; }

  /*! \brief Sets the format used for representing the datetime.
   *
   * This sets the format in the validator.
   *
   * The default format is based on the current WLocale.
   *
   * The format is set and limited to "yyyy-MM-ddThh:mm" when using
   * the native HTML5 control. Changing to another format has
   * no effect.
   *
   * \note The WTimePicker does not support timezones, and
   *       consequentially this widget doesn't either. If supplied, it
   *       is removed.
   *
   * \sa setNativeControl(), WDateTimeValidator::setFormat(), WDateTime::toString(), WTime::toString(), WDate::toString()
   */
  void setFormat(const WT_USTRING& dateTimeFormat);

  /*! \brief Returns the datetime format.
   *
   * \sa setFormat()
   */
  WT_USTRING format() const;

  /*! \brief Sets the lower limit of the valid datetime range.
   *
   * This sets the lower limit of the valid datetime range in the
   * datetime validator.
   *
   * \sa WDateTimeValidator::setBottom()
   */
  void setBottom(const WDateTime& bottom);

  /*! \brief Returns the lower limit of the valid datetime range.
   *
   * \sa setBottom()
   */
  WDateTime bottom() const;

  /*! \brief Sets the upper limit of the valid datetime range.
   *
   * This sets the upper limit of the valid datetime range in the
   * datetime validator.
   *
   * \sa WDateTimeValidator::setTop()
   */
  void setTop(const WDateTime& top);

  /*! \brief Returns the upper limit of the valid datetime range.
   *
   * \sa setTop()
   */
  WDateTime top() const;

  /*! \brief Browses to the same month in the previous year.
   *
   * Displays the same month in the previous year.
   * This does not affect the selection.
   *
   * This will not affect a native widget.
   *
   * \sa WCalendar::browseToPreviousYear()
   */
  void browseToPreviousYear();

  /*! \brief Browses to the previous month.
   *
   * Displays the previous month.
   * This does not affect the selection.
   *
   * This will not affect a native widget.
   *
   * \sa WCalendar::browseToPreviousMonth()
   */
  void browseToPreviousMonth();

  /*! \brief Browses to the same month in the next year.
   *
   * Displays the same month in the next year.
   * This does not change the current selection.
   *
   * This will not affect a native widget.
   *
   * \sa WCalendar::browseToNextYear()
   */
  void browseToNextYear();

  /*! \brief Browses to the next month.
   *
   * Displays the next month. This does not affect the selection.
   *
   * This will not affect a native widget.
   *
   * \sa WCalendar::browseToNextMonth()
   */
  void browseToNextMonth();

  /*! \brief Browses to a date.
   *
   * Displays the month which contains the given date.
   * This does not change the current selection.
   *
   * This will not affect a native widget.
   *
   * \sa WCalendar::browseTo()
   */
  void browseTo(const WDate& date);

  /*! \brief Returns the current year displayed.
   *
   * Returns the year that is currently displayed.
   *
   * This will not work on a native widget.
   *
   * \sa WCalendar::currentYear()
   */
  int currentYear() const;

  /*! \brief Returns the current month displayed.
   *
   * Returns the month (1-12) that is currently displayed.
   *
   * This will not work on a native widget.
   *
   * \sa WCalendar::currentMonth()
   */
  int currentMonth() const;

  /*! \brief Sets the horizontal header format.
   *
   * The default horizontal header format is
   * \link Wt::CalendarHeaderFormat::ShortDayNames \endlink
   *
   * This will have no effect on a native widget.
   *
   * \sa WCalendar::SetHorizontalHeaderFormat()
   */
  void setHorizontalHeaderFormat(CalendarHeaderFormat format);

  /*! \brief Returns the horizontal header format.
   *
   * This will not work on a native widget.
   *
   * \sa setHoritonalHeaderFormat()
   */
  CalendarHeaderFormat horizontalHeaderFormat();

  /*! \brief Sets the first dat of the week.
   *
   * Possible values are 1 to 7. The default value is 1 ("Monday").
   *
   * This will have no effect on a native widget.
   *
   * \sa WCalendar::setFirstDayOfWeek()
   */
  void setFirstDayOfWeek(int dayOfWeek);

  /*! \brief Sets the step size for the hours
   *
   * This only impacts the time sections of the widget.
   *
   * This will have no effect on a native widget.
   *
   * \sa hourStep(), WTimeEdit::setHourStep()
   */
  void setHourStep(int step);

  /*! \brief Returns the step size for the hours
   *
   * This will have no effect on a native widget.
   *
   * \sa setHourStep(), WTimeEdit::hourStep()
   */
  int hourStep() const;

  /*! \brief Sets the step size for the minutes
   *
   * This only impacts the time sections of the widget.
   *
   * This will have no effect on a native widget.
   *
   * \sa minuteStep(), WTimeEdit::setMinuteStep()
   */
  void setMinuteStep(int step);

  /*! \brief Returns the step size for the minutes
   *
   * This will have no effect on a native widget.
   *
   * \sa setMinuteStep(), WTimeEdit::minuteStep()
   */
  int minuteStep() const;

  /*! \brief Sets the step size for the seconds
   *
   * This only impacts the time sections of the widget.
   *
   * This will have no effect on a native widget.
   *
   * \sa secondStep(), WTimeEdit::setSecondStep()
   */
  void setSecondStep(int step);

  /*! \brief Returns the step size for the seconds
   *
   * This will have no effect on a native widget.
   * \sa setSecondStep(), WTimeEdit::secondStep();
   */
  int secondStep() const;

  /*! \brief Sets the step size for the milliseconds
   *
   * This only impacts the time sections of the widget.
   *
   * This will have no effect on a native widget.
   *
   * \sa millisecondStep(), WTimeEdit::setMillisecondStep()
   */
  void setMillisecondStep(int step);

  /*! \brief Returns the step size for the milliseconds
   *
   * This will have no effect on a native widget.
   *
   * \sa setMillisecondStep(), WTimeEdit::millisecondStep()
   */
  int millisecondStep() const;

  /*! \brief Enables or disables wraparound for the time
   *
   * Wraparound is enabled by default. This will ensure
   * that the widgets that selects the units of time will
   * wrap around to their lowest possible value when
   * exceeding their maximum value.
   *
   * This will have no effect on a native widget.
   *
   * \sa wrapAroundEnabled()
   */
  void setWrapAroundEnabled(bool rollover);

  /*! \brief Returns whether wraparound is enabled
   *
   * This will have no effect on a native widget.
   *
   * \sa setWrapAroundEnabled()
   */
  bool wrapAroundEnabled() const;

  void setHidden(bool hidden, const WAnimation& animation = WAnimation()) override;

  void load() override;
  void refresh() override;

protected:
  void render(WFlags<RenderFlag> flags) override;

  //! Sets the value from the line edit to the calenbdar and time picker.
  virtual void setFromLineEdit();
  void updateDom(DomElement& element, bool all) override;
  void validatorChanged() override;
  std::string type() const noexcept override;

private:
  std::unique_ptr<WPopupWidget> uPopup_;
  std::unique_ptr<WCalendar> uCalendar_;
  WCalendar *calendar_;
  std::unique_ptr<WTimePicker> uTimePicker_;
  WTimePicker *timePicker_;
  bool customFormat_;
  bool nativeControl_;

  void defineJavaScript();
  void connectJavaScript(Wt::EventSignalBase& s, const std::string& methodName);
  void setFocusTrue();

  void setFromTimePicker();
  void setFromCalendar();

};

}

#endif // WDATETIME_EDIT_H_
