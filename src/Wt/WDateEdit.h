// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDATE_EDIT_H_
#define WDATE_EDIT_H_

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
 * line edit. This also makes the implementation ready for a native
 * HTML5 control.
 */
class WT_API WDateEdit : public WLineEdit
{
public:
  /*! \brief Creates a new date edit.
   */
  WDateEdit();

  virtual ~WDateEdit();

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
   */
  virtual std::shared_ptr<WDateValidator> dateValidator() const;

  /*! \brief Sets the format used for representing the date.
   *
   * This sets the format in the validator.
   *
   * The default format is based on the current WLocale.
   *
   * \sa WDateValidator::setFormat()
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
  WCalendar *calendar() const { return calendar_; }

  /*! \brief Hide/unhide the widget.
   */
  virtual void setHidden(bool hidden,
                         const WAnimation& animation = WAnimation()) override;

  virtual void load() override;
  virtual void refresh() override;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void propagateSetEnabled(bool enabled) override;

  /*! \brief Sets the value from the calendar to the line edit.
   */
  virtual void setFromCalendar();

  /*! \brief Sets the value from the line edit to the calendar.
   */
  virtual void setFromLineEdit();

private:
  std::unique_ptr<WPopupWidget> popup_;
  std::unique_ptr<WCalendar> uCalendar_;
  WCalendar *calendar_;
  bool customFormat_;

  void defineJavaScript();
  void connectJavaScript(Wt::EventSignalBase& s, const std::string& methodName);
  void setFocusTrue();
};

}

#endif // WDATE_EDIT_H_
