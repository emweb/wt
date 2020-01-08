// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCALENDAR_H_
#define WCALENDAR_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WDate.h>
#include <set>

namespace Wt {

/*! \brief The calendar header format.
 */
enum class CalendarHeaderFormat { 
  SingleLetterDayNames,  //!< First letter of a day (e.g. 'M' for Monday)
  ShortDayNames,         //!< First 3 letters of a day (e.g. 'Mon' for Monday)
  LongDayNames           //!< Full day name
    // NoHorizontalHeader  //No horizontal header (not yet implemented)
};

class WComboBox;
class WInPlaceEdit;
class WTemplate;

/*! \class WCalendar Wt/WCalendar.h Wt/WCalendar.h
 *  \brief A calendar.
 *
 * The calendar provides navigation by month and year, and indicates the
 * current day.
 *
 * You can listen for single click or double click events on a
 * calendar cell using the clicked() and activated() methods.
 *
 * The calendar may be configured to allow selection of single or
 * multiple days using setSelectionMode(), and you may listen for
 * changes in the selection using the selectionChanged()
 * signals. Selection can also be entirely disabled in which case you
 * can implement your own selection handling by listening for cell
 * click events.
 *
 * Cell rendering may be customized by reimplementing renderCell().
 *
 * Internationalization is provided by the internationalization
 * features of the Wt::WDate class.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WDate today = Wt::WDate::currentDate();
 *
 * Wt::WCalendar *calendar = addWidget(std::make_unique<Wt::WCalendar>());
 * calendar->browseTo(today.addMonths(1));
 * calendar->select(today.addMonths(1).addDays(3));
 * calendar->selected().connect(this, &MyWidget::daySelected);
 * \endcode
 * \endif
 *
 * Here is a snapshot, taken on 19/01/2010 (shown as
 * today), and 14/01/2010 currently selected.
 * <TABLE border="0" align="center"> <TR> <TD> 
 * \image html WCalendar-default-1.png "WCalendar with default look" 
 * </TD> <TD> 
 * \image html WCalendar-polished-1.png "WCalendar with polished look" 
 * </TD> </TR> </TABLE>
 *
 */
class WT_API WCalendar : public WCompositeWidget
{
public:
  /*! \brief Typedef for enum Wt::CalendarHeaderFormat */
  typedef CalendarHeaderFormat HeaderFormat;

  /*! \brief Creates a new calendar.
   *
   * Constructs a new calendar with English day/month names.  The
   * calendar shows the current day, and has an empty selection.
   */
  WCalendar();

  /*! \brief Sets the selection mode.
   *
   * The default selection mode is
   * \link Wt::SingleSelection SingleSelection\endlink.
   */
  void setSelectionMode(SelectionMode mode);

  /*! \brief Browses to the same month in the previous year.
   *
   * Displays the same month in the previous year. This does not
   * affect the selection.
   *
   * This will emit the currentPageChanged() singal.
   */
  void browseToPreviousYear();

  /*! \brief Browses to the previous month.
   *
   * Displays the previous month. This does not affect the selection.
   *
   * This will emit the currentPageChanged() singal.
   */
  void browseToPreviousMonth();

  /*! \brief Browses to the same month in the next year.
   *
   * Displays the same month in the next year. This does not change
   * the current selection.
   *
   * This will emit the currentPageChanged() singal.
   */
  void browseToNextYear();

  /*! \brief Browses to the next month.
   *
   * Displays the next month. This does not change the current selection.
   *
   * This will emit the currentPageChanged() singal.
   */
  void browseToNextMonth();

  /*! \brief Browses to a date.
   *
   * Displays the month which contains the given date. This does not change
   * the current selection.
   *
   * This will emit the currentPageChanged() signal if another month
   * is displayed.
   */
  void browseTo(const WDate& date);

  /*! \brief Returns the current month displayed
   *
   * Returns the month (1-12) that is currently displayed.
   */
  int currentMonth() const { return currentMonth_; }

  /*! \brief Returns the current year displayed
   *
   * Returns the year that is currently displayed.
   */
  int currentYear() const { return currentYear_; }

  /*! \brief Clears the current selection.
   *
   * Clears the current selection. Will result in a selection() that is
   * empty().
   */
  void clearSelection();

  /*! \brief Selects a date.
   *
   * Select one date. Both in single or multiple selection mode, this results
   * in a selection() that contains exactly one date.
   */
  void select(const WDate& date);

  /*! \brief Selects multiple dates.
   *
   * Select multiple dates. In multiple selection mode, this results
   * in a selection() that contains exactly the given dates. In single
   * selection mode, at most one date is set.
   */
  void select(const std::set<WDate>& dates);

  /*! \brief Sets the horizontal header format.
   *
   * The default horizontal header format is CalendarHeaderFormat::ShortDayNames.
   */
  void setHorizontalHeaderFormat(CalendarHeaderFormat format);

  /*! \brief Returns the horizontal header format.
   *
   * \sa setHorizontalHeaderFormat()
   */
  CalendarHeaderFormat horizontalHeaderFormat() {
    return horizontalHeaderFormat_;
  }

  /*! \brief Sets the first day of the week.
   *
   * Possible values are 1 to 7. The default value is 1 ("Monday").
   */
  void setFirstDayOfWeek(int dayOfWeek);

  /*! \brief Returns the current selection.
   *
   * Returns the set of dates currently selected. In single selection mode,
   * this set contains 0 or 1 dates.
   */
  const std::set<WDate>& selection() const { return selection_; }

  /*! \brief %Signal emitted when the user changes the selection.
   *
   * Emitted after the user has changed the current selection.
   */
  Signal<>& selectionChanged() { return selectionChanged_; }

  /*! \brief %Signal emitted when the user double-clicks a date.
   *
   * You may want to connect to this signal to treat a double click
   * as the selection of a date.
   */
  Signal<WDate>& activated() { return activated_; }

  /*! \brief %Signal emitted when the user clicks a date.
   *
   * You may want to connect to this signal if you want to provide a
   * custom selection handling.
   */
  Signal<WDate>& clicked() { return clicked_; } 

  /*! \brief %Signal emitted when the current month is changed.
   *
   * The method is emitted both when the change is done through the
   * user interface or via the public API. The two parameters are
   * respectively the new year and month.
   */
  Signal<int, int>& currentPageChanged() { return currentPageChanged_; }

  /*! \brief Configures the calendar to use single click for activation
   *
   * By default, double click will trigger activate(). Use this method
   * if you want a single click to trigger activate() (and the now
   * deprecated selected() method). This only applies to a
   * single-selection calendar.
   *
   * If selectionMode() is set to \link Wt::SingleSelection SingleSelection\endlink,
   * this will cause the selection to change on a single click instead of a double click.
   *
   * Instead of enabling single click, you can also listen to the clicked()
   * signal to process a single click.
   *
   * \sa setSelectionMode()
   */
  void setSingleClickSelect(bool single);

  /*! \brief Sets the bottom of the valid date range.
   *
   * \if cpp
   * The default is a null date constructed using WDate().
   * \elseif java 
   * The default bottom is null.
   * \endif
   */
  void setBottom(const WDate& bottom);

  /*! \brief Returns the bottom date of the valid range.
   */
  const WDate& bottom() const { return bottom_; }

  /*! \brief Sets the top of the valid date range.
   *
   * \if cpp
   * The default is a null date constructed using WDate().
   * \elseif java 
   * The default top is null.
   * \endif
   */
  void setTop(const WDate& top);

  /*! \brief Returns the top date of the valid range.
   */
  const WDate& top() const { return top_; }

  virtual void load() override;

protected:
  virtual void render(WFlags<RenderFlag> renderFlags) override;
  
  /*! \brief Creates or updates a widget that renders a cell.
   *
   * The default implementation creates a WText
   *
   * You may want to reimplement this method if you wish to customize
   * how a cell is rendered. When \p widget is \c 0, a new widget
   * should be created and returned. Otherwise, you may either modify
   * the passed \p widget, or return a new widget. If you return a new
   * widget, the prevoius widget will be deleted.
   */
  virtual WWidget* renderCell(WWidget* widget, const WDate& date);

  /*! \brief Returns whether a date is selected.
   *
   * This is a convenience method that can be used when reimplementing
   * renderCell().
   */
  bool isSelected(const WDate& date) const;

  virtual void enableAjax() override;

private:
  SelectionMode     selectionMode_;
  bool              singleClickSelect_;
  int               currentYear_;
  int               currentMonth_;
  CalendarHeaderFormat horizontalHeaderFormat_;
  int               firstDayOfWeek_;
  std::set<WDate>   selection_;
  bool              needRenderMonth_;

  Signal<>          selectionChanged_;
  Signal<WDate>     activated_;
  Signal<WDate>     clicked_;
  Signal<int, int>  currentPageChanged_;

  WDate             bottom_, top_;

  struct Coordinate {
    int i, j;

    Coordinate() : i(0), j(0) { }
    Coordinate(int x, int y) { i = x; j = y; }
  };

  WTemplate *impl_;
  WComboBox *monthEdit_;
  WInPlaceEdit *yearEdit_;

  void create();
  void renderMonth();

  void emitCurrentPageChanged();

  void monthChanged(int newMonth);
  void yearChanged(WString newYear);
  WDate dateForCell(int week, int dayOfWeek);

  void selectInCurrentMonth(const WDate& d);

  bool isInvalid(const WDate& d);
  void cellClicked(Coordinate c);
  void cellDblClicked(Coordinate c);
};

}

#endif // WCALENDAR_H_
