// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDATE_EDIT_H_
#define WDATE_EDIT_H_

#include "Wt/WCalendar.h"
#include <Wt/WDateValidator.h>
#include <Wt/WLineEdit.h>

namespace Wt {

/*! \class WDateEdit Wt/WDateEdit.h Wt/WDateEdit.h
 *  \brief A date edit.
 *
 * A date picker is a line edit with support for date entry (using an
 * icon and a calendar).
 *
 * A WDateValidator is used to validate date entry.
 *
 * In many cases, it provides a more convenient implementation of a
 * date picker compared to WDatePicker since it is implemented as a
 * line edit, and a WDateEdit can be configured as a
 * \link setNativeControl() native HTML5 control\endlink.
 *
 * When the native HTML5 control is used, the format is limited to
 * \c yyyy-MM-dd. Changing to another format has no effect.
 */
class WT_API WDateEdit : public WLineEdit
{
public:
  /*! \brief Creates a new date edit.
   */
  WDateEdit();

  virtual ~WDateEdit();

  /*! \brief Changes whether a native HTML5 control is used.
   *
   * When enabled the browser's native date input
   * (<input type="date">) will be used, if available. This should
   * provide a better experience on mobile browsers.
   * This option is set to false by default.
   *
   * Setting native control to true limits the format to "yyyy-MM-dd".
   * Note that this is the format that the widget returns, not the
   * format the user will see. This format is decided by the browser
   * based on the user's locale.
   *
   * There is no support for changing whether a native control is
   * used after the widget is rendered.
   *
   * \sa nativeControl()
   */
  void setNativeControl(bool nativeControl);

  /*! \brief Returns whether a native HTML5 control is used.
   *
   * Taking into account the preference for a native control,
   * configured using setNativeControl(bool nativeControl), this method
   * returns whether a native control is actually being used.
   *
   * \sa setNativeControl()
   */
  WT_NODISCARD bool nativeControl() const noexcept { return nativeControl_; }

  /*! \brief Sets the date.
   *
   * Does nothing if the current date is \p Null.
   *
   * \sa date()
   */
  void setDate(const WDate& date);

  /*! \brief Returns the date.
   *
   * Reads the current date.
   *
   * \if cpp
   * Returns an invalid date (for which WDate::isValid() returns
   * \c false) if the date could not be parsed using the current
   * format(). <br>
   * \elseif java
   * Returns \c null if the date could not be parsed using the current
   * format(). <br>
   * \endif
   *
   * \sa setDate(), WDate::fromString(), WLineEdit::text()
   */
  WDate date() const;

  /*! \brief Returns the validator.
   *
   * Most of the configuration of the date edit is stored in the
   * validator.
   *
   * \note Using the validator to change the format while a
   * native control is being used will break the native control. If a
   * native control is used, do not call WDateValidator()::setFormat(),
   * instead use WDateEdit::setFormat().
   *
   * \sa WDateValidator::WDateValidator(), setFormat()
   */
  virtual std::shared_ptr<WDateValidator> dateValidator() const;

  /*! \brief Sets the format used for representing the date.
   *
   * This sets the format in the validator.
   *
   * The default format is based on the current WLocale.
   *
   * The format is set and limited to "yyyy-MM-dd" when using
   * the native HTML5 control. Changing to another format has
   * no effect.
   *
   * \sa WDateValidator::setFormat(), setNativeControl()
   */
  void setFormat(const WT_USTRING& format);

  /*! \brief Returns the format.
   *
   * \sa setFormat()
   */
  WT_USTRING format() const;

  /*! \brief Sets the lower limit of the valid date range.
   *
   * This sets the lower limit of the valid date range in the
   * validator.
   *
   * \sa WDateValidator::setBottom()
   */
  void setBottom(const WDate& bottom);

  /*! \brief Returns the lower limit of the valid date range.
   *
   * \sa setBottom()
   */
  WDate bottom() const;

  /*! \brief Sets the upper limit of the valid date range.
   *
   * This sets the upper limit of the valid date range in the
   * validator.
   *
   * \sa WDateValidator::setTop()
   */
  void setTop(const WDate& top);

  /*! \brief Returns the upper limit of the valid range.
   *
   * \sa setTop()
   */
  WDate top() const;

  /*! \brief Returns the calendar widget.
   *
   * The calendar may be 0 (e.g. when using a native date entry
   * widget).
   */
  WCalendar *calendar() const { return oCalendar_.get(); }

  /*! \brief Hide/unhide the widget.
   */
  virtual void setHidden(bool hidden,
                         const WAnimation& animation = WAnimation()) override;

  virtual void load() override;
  virtual void refresh() override;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void propagateSetEnabled(bool enabled) override;
  void validatorChanged() override;
  WT_NODISCARD std::string type() const noexcept override;
  void updateDom(DomElement& element, bool all) override;

  /*! \brief Sets the value from the calendar to the line edit.
   */
  virtual void setFromCalendar();

  /*! \brief Sets the value from the line edit to the calendar.
   */
  virtual void setFromLineEdit();

private:
  std::unique_ptr<WPopupWidget> popup_;
  std::unique_ptr<WCalendar> uCalendar_;
  Wt::Core::observing_ptr<WCalendar> oCalendar_;
  bool customFormat_;
  bool nativeControl_;

  void init();
  void defineJavaScript();
  void connectJavaScript(Wt::EventSignalBase& s, const std::string& methodName);
  void setFocusTrue();
};

}

#endif // WDATE_EDIT_H_
