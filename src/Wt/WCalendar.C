/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCalendar"

#include <boost/date_time/gregorian/gregorian.hpp>
using namespace boost::gregorian;

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WComboBox"
#include "Wt/WContainerWidget"
#include "Wt/WInPlaceEdit"
#include "Wt/WLineEdit"
#include "Wt/WSignalMapper"
#include "Wt/WTable"
#include "Wt/WText"

namespace Wt {

WCalendar::WCalendar(bool i18n, WContainerWidget *parent)
  : WCompositeWidget(parent),
    selectionChanged(this),
    selected(this),
    i18n_(i18n),
    multipleSelection_(false)
{
  WDate currentDay = WDate::currentDate();

  currentYear_ = currentDay.year();
  currentMonth_ = currentDay.month();

  create();
}

void WCalendar::setMultipleSelection(bool multiple)
{
  if (multiple != multipleSelection_) {
    if (!multiple && selection_.size() > 1) {
      selection_.clear();
      renderMonth();
    }
    multipleSelection_ = multiple;
  }
}

void WCalendar::create()
{
  setImplementation(layout_ = new WContainerWidget());

  const char *CSS_RULES_NAME = "Wt::WCalendar";

  WApplication *app = WApplication::instance();
  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    app->styleSheet().addRule("table.Wt-cal-table ",
			      "border-collapse:separate;"
			      "border-spacing:0pt;"
			      "width: 18em;", CSS_RULES_NAME);
    app->styleSheet().addRule("*.Wt-cal-table td",
			      "color: #003DB8;"
			      "border: 1px solid #E0E0E0;"
			      "cursor: pointer; cursor: hand;"
			      "text-align: center;"
			      "padding: 0.1em 0.2em;");
    app->styleSheet().addRule("*.Wt-cal-table td:hover",
			      "color: #FFFFFF;"
			      "border:1px solid #FF9900;"
			      "background-color: #FF9900;");
    app->styleSheet().addRule("td.Wt-cal-header, "
			      "td.Wt-cal-header:hover",
			      "color: #666666;"
			      "border: 0px;"
			      "width: 2em;"
			      "background-color: transparent;");
    app->styleSheet().addRule("td.Wt-cal-header-weekend, "
			      "td.Wt-cal-header-weekend:hover",
			      "color: #777777;"
			      "border: 0px;"
			      "width: 2em;"
			      "background-color: transparent;");
    app->styleSheet().addRule("td.Wt-cal-oom, "
			      "td.Wt-cal-oom:hover",
			      "color: #999999;"
			      "cursor: default;"
			//"border: 1px solid transparent;" doesn't work on IE6
			      "border: 0px;"
			      "background-color: transparent;");
    app->styleSheet().addRule("td.Wt-cal-sel",
			      "background-color:#FFF19F;"
			      "border:1px solid #FF9900;");
    app->styleSheet().addRule("td.Wt-cal-now",
			      "border:1px solid #000000;");
    app->styleSheet().addRule("*.Wt-cal-navbutton",
			      "color: #FFFFFF;"
			      "background-color:#6699CC;"
			      "cursor: pointer; cursor: hand;"
			      "margin: 0px 3px;");
    app->styleSheet().addRule("*.Wt-cal-year span",
			      "border: 1px solid transparent;");
    app->styleSheet().addRule("*.Wt-cal-year span:hover",
			      "background-color:#FFFFCC;"
			      "border: 1px solid #CCCCCC;");
  }

  layout_->resize(WLength(18, WLength::FontEm), WLength());

  /*
   * Navigation bar
   */
  WContainerWidget *navigation = new WContainerWidget(layout_);
  navigation->setContentAlignment(AlignCenter);

  WText *prevYear = new WText("<<", PlainText, navigation);
  prevYear->setStyleClass("Wt-cal-navbutton");
  prevYear->clicked.connect(SLOT(this, WCalendar::browseToPreviousYear));

  WText *prevMonth = new WText("<", PlainText, navigation);
  prevMonth->setStyleClass("Wt-cal-navbutton");
  prevMonth->clicked.connect(SLOT(this, WCalendar::browseToPreviousMonth));

  monthEdit_ = new WComboBox(navigation);
  for (unsigned i = 0; i < 12; ++i)
    monthEdit_->addItem(i18n_ 
			? tr(WDate::longMonthName(i+1).toUTF8().c_str())
			: WDate::longMonthName(i+1));
  monthEdit_->activated.connect(SLOT(this, WCalendar::monthChanged));

  yearEdit_ = new WInPlaceEdit("", navigation);
  yearEdit_->lineEdit()->setTextSize(4);
  yearEdit_->setStyleClass("Wt-cal-year");
  yearEdit_->valueChanged.connect(SLOT(this, WCalendar::yearChanged));
 
  WText *nextMonth = new WText(">", PlainText, navigation);
  nextMonth->setStyleClass("Wt-cal-navbutton");
  nextMonth->clicked.connect(SLOT(this, WCalendar::browseToNextMonth));

  WText *nextYear = new WText(">>", PlainText, navigation);
  nextYear->setStyleClass("Wt-cal-navbutton");
  nextYear->clicked.connect(SLOT(this, WCalendar::browseToNextYear));

  /*
   * Calendar table
   */
  calendar_ = new WTable(layout_);
  calendar_->setStyleClass("Wt-cal-table");

  for (unsigned i = 0; i < 7; ++i) {
    new WText(i18n_ 
	      ? tr(WDate::shortDayName(i+1).toUTF8().c_str())
	      : WDate::shortDayName(i+1),
	      calendar_->elementAt(0, i));
    calendar_->elementAt(0, i)
      ->setStyleClass(i < 5 ? "Wt-cal-header" : "Wt-cal-header-weekend");
  }

  renderMonth(true);
}

void WCalendar::renderMonth(bool create)
{
  if (create) {
    cellClickMapper_ = new WSignalMapper<Coordinate>(this);
    cellClickMapper_->mapped.connect(SLOT(this, WCalendar::cellClicked));

    cellDblClickMapper_ = new WSignalMapper<Coordinate>(this);
    cellDblClickMapper_->mapped.connect(SLOT(this, WCalendar::cellDblClicked));
  }

  int m = currentMonth_ - 1;
  if (monthEdit_->currentIndex() != m) {
    monthEdit_->setCurrentIndex(m);
  }

  int y = currentYear_;
  std::string s = boost::lexical_cast<std::string>(y);
  if (yearEdit_->text().toUTF8() != s) {
    yearEdit_->setText(s);
  }

  WDate nowd = WDate::currentDate();
  date now(nowd.year(), nowd.month(), nowd.day());

  // The first line contains the last day of the previous month.
  date d(currentYear_, currentMonth_, 1);
  d -= date_duration(1);
  greg_weekday gw(Monday);
  d = previous_weekday(d, gw);

  for (unsigned i = 0; i < 6; ++i) {
    for (unsigned j = 0; j < 7; ++j) {
      WTableCell *cell = calendar_->elementAt(i+1, j);

      if (create) {
	WInteractWidget *w
	  = new WText(boost::lexical_cast<std::string>(d.day()), cell);

	if (WApplication::instance()->environment().javaScript()) {
	  w = cell; // we cannot wrap a TD in a button !
	}

	cellClickMapper_->mapConnect(w->clicked, Coordinate(i, j));
	cellDblClickMapper_->mapConnect(w->doubleClicked, Coordinate(i, j));
      } else {
	WText *t = dynamic_cast<WText *>(cell->children()[0]);
	t->setText(boost::lexical_cast<std::string>(d.day()));
      }

      WDate date(d.year(), d.month(), d.day());

      std::string styleClass;

      if (isSelected(date))
	styleClass += " Wt-cal-sel";

      if (d.month() != currentMonth_)
	styleClass += " Wt-cal-oom";

      if (d == now)
	styleClass += " Wt-cal-now";

      cell->setStyleClass(styleClass);

      d += date_duration(1);
    }
  }
}

bool WCalendar::isSelected(const WDate& d) const
{
  return selection_.find(d) != selection_.end();
}

void WCalendar::clearSelection()
{
  selection_.clear();

  renderMonth();
}

void WCalendar::select(const WDate& d)
{
  selection_.clear();

  selection_.insert(d);
  renderMonth();
}

void WCalendar::browseTo(const WDate& d)
{
  currentYear_ = d.year();
  currentMonth_ = d.month();

  renderMonth();
}

void WCalendar::select(const std::set<WDate>& dates)
{
  if (multipleSelection_) {
    selection_ = dates;
    renderMonth();
  } else {
    if (dates.empty())
      clearSelection();
    else
      select(*dates.begin());
  }
}

void WCalendar::cellClicked(Coordinate weekday)
{
  date dt = dateForCell(weekday.i, weekday.j);

  selectInCurrentMonth(dt);
}

bool WCalendar::selectInCurrentMonth(const boost::gregorian::date& dt)
{
  if (dt.month() == currentMonth_) {
    WDate d(dt.year(), dt.month(), dt.day());

    if (multipleSelection_) {
      if (isSelected(d))
	selection_.erase(d);
      else
	selection_.insert(d);

      selectionChanged.emit();
      renderMonth();

    } else {
      selection_.clear();
      selection_.insert(d);

      selectionChanged.emit();
      renderMonth();
    }

    return true;
  } else
    return false;
}

void WCalendar::cellDblClicked(Coordinate weekday)
{
  date dt = dateForCell(weekday.i, weekday.j);  

  if (selectInCurrentMonth(dt))
    if (!multipleSelection_)
      selected.emit();
}

date WCalendar::dateForCell(int week, int dayOfWeek)
{
  date d(currentYear_, currentMonth_, 1);
  d -= date_duration(1);
  greg_weekday gw(Monday);
  d = previous_weekday(d, gw);

  d += date_duration(week * 7 + dayOfWeek);

  return d;
}

void WCalendar::browseToPreviousYear()
{
  --currentYear_;

  renderMonth();
}

void WCalendar::browseToPreviousMonth()
{
  if (--currentMonth_ == 0) {
    currentMonth_ = 12;
    --currentYear_;
  }

  renderMonth();
}

void WCalendar::browseToNextYear()
{
  ++currentYear_;

  renderMonth();
}

void WCalendar::browseToNextMonth()
{
  if (++currentMonth_ == 13) {
    currentMonth_ = 1;
    ++currentYear_;
  }

  renderMonth();
}

void WCalendar::monthChanged(int newMonth)
{
  ++newMonth;

  if (currentMonth_ != newMonth
      && (newMonth >= 1 && newMonth <= 12)) {

    currentMonth_ = newMonth;

    renderMonth();
  }
}

void WCalendar::yearChanged(WString yearStr)
{
  try {
    int year = boost::lexical_cast<int>(yearStr);

    if (currentYear_ != year &&
	(year >= 1900 && year <= 2200)) { // ??
      currentYear_ = year;

      renderMonth();
    }
  } catch (boost::bad_lexical_cast&) {
  }
}

}
