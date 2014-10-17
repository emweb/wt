/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WCalendar"

using namespace boost::gregorian;

#include "Wt/WComboBox"
#include "Wt/WInPlaceEdit"
#include "Wt/WLineEdit"
#include "Wt/WLogger"
#include "Wt/WSignalMapper"
#include "Wt/WStringStream"
#include "Wt/WTemplate"
#include "Wt/WText"

#include "WebUtils.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Wt {

LOGGER("WCalendar");

// Because WDate returns days and weeks as WT_USTRING, we need this:
#ifndef WT_TARGET_JAVA
#define DATE_NAME_STR(e) e
#else
#define DATE_NAME_STR(e) WString::fromUTF8(e)
#endif

WCalendar::WCalendar(WContainerWidget *parent)
  : WCompositeWidget(parent),
    selectionChanged_(this),
    activated_(this),
    clicked_(this),
    currentPageChanged_(this)
{
  create();
  impl_->addStyleClass("Wt-calendar");
}

void WCalendar::setMultipleSelection(bool multiple)
{
  setSelectionMode(multiple?ExtendedSelection:SingleSelection);
}

void WCalendar::setSelectionMode(SelectionMode mode) 
{
  if (selectionMode_ != mode) {
    if (mode != ExtendedSelection && selection_.size() > 1) {
      selection_.clear();
      renderMonth();
    }
    selectionMode_ = mode;
  }
}

void WCalendar::setSingleClickSelect(bool single)
{
  singleClickSelect_ = single;
}

void WCalendar::create()
{
  selectionMode_ = SingleSelection;
  singleClickSelect_ = false;
  horizontalHeaderFormat_ = ShortDayNames;
  firstDayOfWeek_ = 1;
  cellClickMapper_ = 0;
  cellDblClickMapper_ = 0;

  WDate currentDay = WDate::currentDate();

  currentYear_ = currentDay.year();
  currentMonth_ = currentDay.month();

  WStringStream text;

  text <<
    "<table class=\"days ${table-class}\" cellspacing=\"0\" cellpadding=\"0\">"
    """<tr>"
    ""  "<th class=\"caption\">${nav-prev}</th>"
    ""  "<th class=\"caption\"colspan=\"5\">${month} ${year}</th>"
    ""  "<th class=\"caption\">${nav-next}</th>"
    """</tr>"
    """<tr>";

  for (int j = 0; j < 7; ++j)
    text <<
      "<th title=\"${t" << j << "}\" scope=\"col\">${d" << j << "}</th>";

  text << "</tr>";

  for (int i = 0; i < 6; ++i) {
    text << "<tr>";
    for (int j = 0; j < 7; ++j)
      text << "<td>${c" << (i * 7 + j) << "}</td>";
    text << "</tr>";
  }

  text << "</table>";

  setImplementation(impl_ = new WTemplate());
  impl_->setTemplateText(WString::fromUTF8(text.str()), XHTMLUnsafeText);
  impl_->setStyleClass("Wt-cal");

  setSelectable(false);

  WText *prevMonth = new WText(tr("Wt.WCalendar.PrevMonth"));
  prevMonth->setStyleClass("Wt-cal-navbutton");
  prevMonth->clicked().connect(this, &WCalendar::browseToPreviousMonth);

  WText *nextMonth = new WText(tr("Wt.WCalendar.NextMonth"));
  nextMonth->setStyleClass("Wt-cal-navbutton");
  nextMonth->clicked().connect(this, &WCalendar::browseToNextMonth);

  monthEdit_ = new WComboBox();
  monthEdit_->setInline(true);
  for (unsigned i = 0; i < 12; ++i)
    monthEdit_->addItem(WDate::longMonthName(i+1));
  monthEdit_->activated().connect(this, &WCalendar::monthChanged);
  monthEdit_->setDisabled(!WApplication::instance()->environment().ajax());

  yearEdit_ = new WInPlaceEdit("");
  yearEdit_->setButtonsEnabled(false);
  yearEdit_->lineEdit()->setTextSize(4);
  yearEdit_->setStyleClass("Wt-cal-year");
  yearEdit_->valueChanged().connect(this, &WCalendar::yearChanged);

  impl_->bindWidget("nav-prev", prevMonth);
  impl_->bindWidget("nav-next", nextMonth);
  impl_->bindWidget("month", monthEdit_);
  impl_->bindWidget("year", yearEdit_);

  setHorizontalHeaderFormat(horizontalHeaderFormat_);
  setFirstDayOfWeek(firstDayOfWeek_);
}

void WCalendar::enableAjax()
{
  WCompositeWidget::enableAjax();
  monthEdit_->enable();
}

void WCalendar::setFirstDayOfWeek(int dayOfWeek)
{
  firstDayOfWeek_ = dayOfWeek;

  for (unsigned i = 0; i < 7; ++i) {
    int day = (i + firstDayOfWeek_ - 1) % 7 + 1;

    WString title = WDate::longDayName(day);
    impl_->bindString("t" + boost::lexical_cast<std::string>(i), 
		      title, 
		      XHTMLUnsafeText);

    WString abbr;
    switch (horizontalHeaderFormat_) {
    case SingleLetterDayNames:
      abbr = WString::fromUTF8(WDate::shortDayName(day).toUTF8().substr(0, 1));
      break;
    case ShortDayNames:
      abbr = WDate::shortDayName(day);
      break;
    case LongDayNames:
      abbr = WDate::longDayName(day);
      break;
    }
   
    impl_->bindString("d" + boost::lexical_cast<std::string>(i), 
		      abbr, 
		      XHTMLUnsafeText);
  }

  renderMonth();
}

void WCalendar::setHorizontalHeaderFormat(HorizontalHeaderFormat format)
{
  std::string d;
  switch (format) {
  case SingleLetterDayNames:
    d = "d1"; break;
  case ShortDayNames:
    d = "d3"; break;
  case LongDayNames:
    d = "dlong"; break;
  default:
    LOG_ERROR("setHorizontalHeaderFormat(): "
	      "improper horizontal header format.");
    format = SingleLetterDayNames;
    d = "d1";
  }

  horizontalHeaderFormat_ = format;

  impl_->bindString("table-class", d, XHTMLUnsafeText);

  setFirstDayOfWeek(firstDayOfWeek_);
}

void WCalendar::setDayOfWeekLength(int chars)
{
  setHorizontalHeaderFormat(chars == 3 ? ShortDayNames : SingleLetterDayNames);
}

void WCalendar::renderMonth()
{
  needRenderMonth_ = true;

  if (isRendered())
    scheduleRender();
}

void WCalendar::render(WFlags<RenderFlag> flags)
{
  if (needRenderMonth_) {
    bool create = cellClickMapper_ == 0;
#ifndef WT_TARGET_JAVA
    char buf[30];
#else
    char *buf;
#endif // WT_TARGET_JAVA

    if (create) {
      // FIXME catch events only on container, and use 'tid' to identify
      // the cell -- in Ajax mode
      cellClickMapper_ = new WSignalMapper<Coordinate>(this);
      cellClickMapper_->mapped().connect(this, &WCalendar::cellClicked);
      cellDblClickMapper_ = new WSignalMapper<Coordinate>(this);
      cellDblClickMapper_->mapped().connect(this, &WCalendar::cellDblClicked);
    }

    int m = currentMonth_ - 1;
    if (monthEdit_->currentIndex() != m)
      monthEdit_->setCurrentIndex(m);

    int y = currentYear_;
    Utils::itoa(y, buf);
    if (yearEdit_->text().toUTF8() != buf)
      yearEdit_->setText(WString::fromUTF8(buf));

    WDate todayd = WDate::currentDate();
    date today(todayd.year(), todayd.month(), todayd.day());

    // The first line contains the last day of the previous month.
    date d(currentYear_, currentMonth_, 1);
    d -= date_duration(1);
 
    greg_weekday gw = firstDayOfWeek_ % 7;
    d = previous_weekday(d, gw);

    for (unsigned i = 0; i < 6; ++i) {
      for (unsigned j = 0; j < 7; ++j) {
	Utils::itoa(i * 7 + j, buf);
	std::string cell = std::string("c") + buf;
	
	WDate date(d.year(), d.month(), d.day());

	WWidget *w = impl_->resolveWidget(cell);
	WWidget *rw = renderCell(w, date);
	impl_->bindWidget(cell, rw);

	WInteractWidget* iw = dynamic_cast<WInteractWidget*>(rw->webWidget());

	if (iw && iw != w) {
	  if (clicked().isConnected()
	      || (selectionMode_ == ExtendedSelection)
	      || (selectionMode_ != ExtendedSelection && singleClickSelect_
		  && activated().isConnected()))
	    cellClickMapper_
	      ->mapConnect(iw->clicked(), Coordinate(i, j));

	  if ((selectionMode_ != ExtendedSelection && !singleClickSelect_
	       && (activated().isConnected() ||
		   selectionChanged().isConnected())))
	    cellDblClickMapper_
	      ->mapConnect(iw->doubleClicked(), Coordinate(i, j));
	}

	d += date_duration(1);
      }
    }

    needRenderMonth_ = false;
  }

  WCompositeWidget::render(flags);
}

WWidget* WCalendar::renderCell(WWidget* widget, const WDate& date)
{
  WText* t = dynamic_cast<WText*>(widget);

  if (!t) {
    t = new WText();
    t->setInline(false);
    t->setTextFormat(PlainText);
  }

#ifndef WT_TARGET_JAVA
    char buf[30];
#else
    char *buf;
#endif // WT_TARGET_JAVA
  Utils::itoa(date.day(), buf);
  t->setText(WString::fromUTF8(buf));

  std::string styleClass;

  if (isInvalid(date))
    styleClass += " Wt-cal-oor";
  else if (date.month() != currentMonth())
    styleClass += " Wt-cal-oom";

  if (isSelected(date))
    styleClass += " Wt-cal-sel";

  WDate currentDate = WDate::currentDate();
  if (date.day() == currentDate.day() && date.month() == currentDate.month() &&
      date.year() == currentDate.year()) {
    if (!isSelected(date))
      styleClass += " Wt-cal-now";
    t->setToolTip("Today");
  } else
    t->setToolTip("");

  t->setStyleClass(styleClass.c_str());

  return t;
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

void WCalendar::select(const WDate& date)
{
  selection_.clear();

  selection_.insert(date);
  renderMonth();
}

void WCalendar::browseTo(const WDate& date)
{
  bool rerender = false;

  if (currentYear_ != date.year()) {
    currentYear_ = date.year();
    rerender = true;
  }

  if (currentMonth_ != date.month()) {
    currentMonth_ = date.month();
    rerender = true;
  }

  if (rerender) {
    emitCurrentPageChanged();
    renderMonth();
  }
}

void WCalendar::select(const std::set<WDate>& dates)
{
  if (selectionMode_ == ExtendedSelection) {
    selection_ = dates;
    renderMonth();
  } else if(selectionMode_ == SingleSelection) {
    if (dates.empty())
      clearSelection();
    else
      select(*dates.begin());
  }
}

void WCalendar::selectInCurrentMonth(const WDate& d)
{
  if (d.month() == currentMonth_ && selectionMode_ != NoSelection) {
    if (selectionMode_ == ExtendedSelection) {
      if (isSelected(d))
	selection_.erase(d);
      else
	selection_.insert(d);
    } else {
      selection_.clear();
      selection_.insert(d);
    }

    renderMonth();
    selectionChanged().emit();
  }
}

bool WCalendar::isInvalid(const WDate& dt)
{
  return ((!bottom_.isNull() && dt < bottom_) ||
          (!top_.isNull() && dt > top_));
}

void WCalendar::cellClicked(Coordinate weekday)
{
  date dt = dateForCell(weekday.i, weekday.j);
  WDate d(dt.year(), dt.month(), dt.day());
  if (isInvalid(d))
    return;

  selectInCurrentMonth(d);
  clicked().emit(d);
  
  if (selectionMode_ != ExtendedSelection && singleClickSelect_)
    activated().emit(d);
}

void WCalendar::cellDblClicked(Coordinate weekday)
{
  date dt = dateForCell(weekday.i, weekday.j);
  WDate d(dt.year(), dt.month(), dt.day());
  if (isInvalid(d))
    return;

  selectInCurrentMonth(d);

  if (selectionMode_ != ExtendedSelection && !singleClickSelect_)
    activated().emit(d);
}

date WCalendar::dateForCell(int week, int dayOfWeek)
{
  date d(currentYear_, currentMonth_, 1);
  d -= date_duration(1);
  greg_weekday gw = firstDayOfWeek_ % 7;
  d = previous_weekday(d, gw);

  d += date_duration(week * 7 + dayOfWeek);

  return d;
}

void WCalendar::emitCurrentPageChanged()
{
  currentPageChanged().emit(currentYear_, currentMonth_);
}

void WCalendar::browseToPreviousYear()
{
  --currentYear_;

  emitCurrentPageChanged();
  renderMonth();
}

void WCalendar::browseToPreviousMonth()
{
  if (--currentMonth_ == 0) {
    currentMonth_ = 12;
    --currentYear_;
  }

  emitCurrentPageChanged();
  renderMonth();
}

void WCalendar::browseToNextYear()
{
  ++currentYear_;

  emitCurrentPageChanged();
  renderMonth();
}

void WCalendar::browseToNextMonth()
{
  if (++currentMonth_ == 13) {
    currentMonth_ = 1;
    ++currentYear_;
  }

  emitCurrentPageChanged();
  renderMonth();
}

void WCalendar::monthChanged(int newMonth)
{
  ++newMonth;

  if (currentMonth_ != newMonth
      && (newMonth >= 1 && newMonth <= 12)) {

    currentMonth_ = newMonth;

    emitCurrentPageChanged();
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

      emitCurrentPageChanged();
      renderMonth();
    }
  } catch (boost::bad_lexical_cast& e) {
  }
}

void WCalendar::setBottom(const WDate& bottom)
{
  if (bottom_ != bottom) {
    bottom_ = bottom;
    renderMonth();
  }
}

void WCalendar::setTop(const WDate& top)
{
  if (top_ != top) {
    top_ = top;
    renderMonth();
  }
}
}
