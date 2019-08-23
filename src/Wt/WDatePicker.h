// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDATEPICKER_H_
#define WDATEPICKER_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WDate.h>
#include <Wt/WJavaScriptSlot.h>
#include <set>

namespace Wt {

class WCalendar;
class WInteractWidget;
class WLineEdit;
class WTemplate;

/*! \class WDatePicker Wt/WDatePicker.h Wt/WDatePicker.h
 *  \brief A date picker.
 *
 * A date picker shows a line edit and an icon which when clicked
 * popups a WCalendar for editing the date. Any date entered in the
 * line edit is reflected in the calendar, and vice-versa.
 *
 * Each of these widgets may be accessed individually (lineEdit(),
 * calendar(), and displayWidget()) and there is a constructor that
 * allows you to specify an existing line edit and display widget.
 * 
 * The date format used by default is <tt>"dd/MM/yyyy"</tt> and can be
 * changed using setFormat(). At any time, the date set may be read
 * using date(), or can be changed using setDate().
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WDatePicker *picker = addWidget(std::make_unique<Wt::WDatePicker>());
 * picker->setFormat("dd-MM-yyyy");
 * picker->setDate(Wt::WDate(1976, 6, 14));
 * \endcode
 * \endif
 *
 * <h3>i18n</h3>
 *
 * Internationalization of WDatePicker is mostly handled through
 * the internationalization mechanism of WDate.
 */
class WT_API WDatePicker : public WCompositeWidget
{
public:
  /*! \brief Create a new date picker.
   *
   * This constructor creates a line edit with an icon that leads to a
   * popup calendar. A WDateValidator is configured for the line edit.
   */
  WDatePicker();

  /*! \brief Create a new date picker for a line edit.
   *
   * This constructor creates an icon that leads to a popup calendar.
   *
   * The \p forEdit argument is the lineEdit that works in conjunction
   * with the date picker. This widget does not become part of the
   * date picker, and may be located anywhere else.
   */
  WDatePicker(WLineEdit *forEdit);

  /*! \brief Create a new date picker for existing line edit and with custom
   *         display widget.
   *
   * The \p displayWidget is a button or image which much be
   * clicked to open the date picker.
   *
   * The \p forEdit argument is the lineEdit that works in
   * conjunction with the date picker.
   */
  WDatePicker(std::unique_ptr<WInteractWidget> displayWidget,
	      WLineEdit *forEdit);

  /*! \brief Destructor.
   */
  ~WDatePicker();

  /*! \brief Returns the validator.
   *
   * Most of the configuration of the date edit is stored in the
   * validator.
   */
  virtual std::shared_ptr<WDateValidator> dateValidator() const;

  /*! \brief Sets the format used for parsing or writing the date in
   *         the line edit.
   *
   * Sets the format used for representing the date in the line edit.
   * If the line edit has a WDateValidator configured for it, then also
   * there the format is updated.
   *
   * The default format is <tt>'dd/MM/yyyy'</tt>.
   *
   * \sa format(), WDate::toString()
   */
  void setFormat(const WT_USTRING& format);

  /*! \brief Returns the format.
   *
   * \sa setFormat()
   */
  const WT_USTRING& format() const { return format_; }

  /*! \brief The calendar widget.
   *
   * Returns the calendar widget.
   */
  WCalendar *calendar() const { return calendar_; }

  /*! \brief The line edit.
   *
   * Returns the line edit which works in conjunction with this date
   * picker.
   */
  WLineEdit *lineEdit() const { return forEdit_; }

  /*! \brief The display widget.
   *
   * Returns the icon which activates the popup.
   */
  WInteractWidget *displayWidget() const { return displayWidget_; }

  /*! \brief The popup widget.
   *
   * Returns the popup widget that contains the calendar. 
   */
  WPopupWidget *popupWidget() const { return popup_.get(); }

  /*! \brief The current date.
   *
   * Reads the current date from the lineEdit().
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

  /*! \brief Sets the current date.
   *
   * Does nothing if the current date is \p Null.
   *
   * \sa date()
   */
  void setDate(const WDate& date);

  /*! \brief Sets whether the widget is enabled.
   *
   * This is the oppositie of setDisabled().
   */
  void setEnabled(bool enabled);

  virtual void setDisabled(bool disabled) override;

  /*! \brief Hide/unhide the widget.
   */
  virtual void setHidden(bool hidden,
                         const WAnimation& animation = WAnimation()) override;

  /*! \brief Sets the bottom of the valid date range.
   */
  void setBottom(const WDate& bottom);

  /*! \brief Returns the bottom date of the valid range.
   */
  WDate bottom() const;
  
  /*! \brief Sets the top of the valid date range.
   */
  void setTop(const WDate& top);

  /*! \brief Returns the top date of the valid range.
   */
  WDate top() const;

  /*! \brief %Signal emitted when the value has changed.
   *
   * This signal is emitted when a new date has been entered (either
   * through the line edit, or through the calendar popup).
   */
  Signal<>& changed() { return changed_; }

  /*! \brief Shows or hides the popup.
   */
  void setPopupVisible(bool visible);

  /*! \brief A %signal which indicates that the popup has been closed.
   *
   * The signal is only fired when the popup has been closed by the
   * user.
   */
  Signal<>& popupClosed() { return popupClosed_; }

protected:
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  WT_USTRING format_;
  WInteractWidget *displayWidget_;
  WLineEdit *forEdit_;

  WContainerWidget *layout_;
  std::unique_ptr<WPopupWidget> popup_;
  WCalendar *calendar_;

  Signal<> popupClosed_, changed_;
  JSlot positionJS_;

  void createDefault(WLineEdit *forEdit);
  void create(std::unique_ptr<WInteractWidget> displayWidget,
	      WLineEdit *forEdit);

  void setFromCalendar();
  void setFromLineEdit();
  void onPopupHidden();
};

}

#endif // WDATEPICKER_H_
