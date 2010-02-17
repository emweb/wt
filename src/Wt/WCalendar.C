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
#include "Wt/WTemplate"
#include "Wt/WText"

#include "Utils.h"

namespace Wt {

// Because WDate returns days and weeks as WT_USTRING, we need this:
#ifndef WT_TARGET_JAVA
#define DATE_NAME_STR(e) e
#else
#define DATE_NAME_STR(e) WString::fromUTF8(e)
#endif

WCalendar::WCalendar(WContainerWidget *parent)
  : WCompositeWidget(parent),
    i18n_(false),
    selectionChanged_(this),
    selected_(this)
{
  create();
}


WCalendar::WCalendar(bool i18n, WContainerWidget *parent)
  : WCompositeWidget(parent),
    i18n_(i18n),
    selectionChanged_(this),
    selected_(this)
{
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

void WCalendar::setSingleClickSelect(bool single)
{
  singleClickSelect_ = single;
}

void WCalendar::create()
{
  multipleSelection_ = false;
  singleClickSelect_ = false;
  dayOfWeekChars_ = 3;
  firstDayOfWeek_ = 1;
  cellClickMapper_ = 0;
  cellDblClickMapper_ = 0;

  WDate currentDay = WDate::currentDate();

  currentYear_ = currentDay.year();
  currentMonth_ = currentDay.month();

  std::stringstream text;

  text <<
    "<table class=\"${table-class}\" cellspacing=\"0\" cellpadding=\"0\">"
    """<caption>"
    ""  "${nav-prev} ${month} ${year} ${nav-next}"
    """</caption>"
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

  WText *prevMonth = new WText("«", PlainText);
  prevMonth->setStyleClass("Wt-cal-navbutton");
  prevMonth->clicked().connect(SLOT(this, WCalendar::browseToPreviousMonth));

  WText *nextMonth = new WText("»", PlainText);
  nextMonth->setStyleClass("Wt-cal-navbutton");
  nextMonth->clicked().connect(SLOT(this, WCalendar::browseToNextMonth));

  monthEdit_ = new WComboBox();
  for (unsigned i = 0; i < 12; ++i)
    monthEdit_->addItem(i18n_
			? tr(WDate::longMonthName(i+1).toUTF8().c_str())
			: DATE_NAME_STR(WDate::longMonthName(i+1)));
  monthEdit_->activated().connect(SLOT(this, WCalendar::monthChanged));

  yearEdit_ = new WInPlaceEdit("");
  yearEdit_->setButtonsEnabled(false);
  yearEdit_->lineEdit()->setTextSize(4);
  yearEdit_->setStyleClass("Wt-cal-year");
  yearEdit_->valueChanged().connect(SLOT(this, WCalendar::yearChanged));

  impl_->bindWidget("nav-prev", prevMonth);
  impl_->bindWidget("nav-next", nextMonth);
  impl_->bindWidget("month", monthEdit_);
  impl_->bindWidget("year", yearEdit_);

  setDayOfWeekLength(dayOfWeekChars_);
  setFirstDayOfWeek(firstDayOfWeek_);
}

void WCalendar::setFirstDayOfWeek(int dayOfWeek)
{
  firstDayOfWeek_ = dayOfWeek;

  for (unsigned i = 0; i < 7; ++i) {
    int day = (i + firstDayOfWeek_ - 1) % 7 + 1;

    WString title = i18n_ ? tr(WDate::longDayName(day).toUTF8())
      : DATE_NAME_STR(WDate::longDayName(day));
    impl_->bindString("t" + boost::lexical_cast<std::string>(i), title, XHTMLUnsafeText);

    WString abbr = i18n_ ? tr(WDate::shortDayName(day).toUTF8())
      : DATE_NAME_STR(WDate::shortDayName(day));

    if (dayOfWeekChars_ != 3)
      abbr = WString::fromUTF8(abbr.toUTF8().substr(0, 1));

    impl_->bindString("d" + boost::lexical_cast<std::string>(i), abbr, XHTMLUnsafeText);
  }

  renderMonth();
}

void WCalendar::setDayOfWeekLength(int chars)
{
  dayOfWeekChars_ = chars == 3 ? 3 : 1;

  impl_->bindString("table-class",
		    "d" + boost::lexical_cast<std::string>(dayOfWeekChars_), XHTMLUnsafeText);

  setFirstDayOfWeek(firstDayOfWeek_);
}

void WCalendar::renderMonth()
{
  needRenderMonth_ = true;

  if (isRendered())
    askRerender();
}

void WCalendar::render()
{
  if (needRenderMonth_) {
    bool create = cellClickMapper_ == 0;

#ifndef WT_TARGET_JAVA
    char buf[30];
#else
    char *buf;
#endif // WT_TARGET_JAVA

    if (create) {
      cellClickMapper_ = new WSignalMapper<Coordinate>(this);
      cellClickMapper_->mapped().connect(SLOT(this, WCalendar::cellClicked));

      if (!singleClickSelect_) {
	cellDblClickMapper_ = new WSignalMapper<Coordinate>(this);
	cellDblClickMapper_->mapped().connect(SLOT(this,
						   WCalendar::cellDblClicked));
      }
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

	WText *t = dynamic_cast<WText *>(impl_->resolveWidget(cell));

	if (!t) {
	  t = new WText();
	  t->setInline(false);
	  t->setTextFormat(PlainText);
	  impl_->bindWidget(cell, t);

	  cellClickMapper_->mapConnect(t->clicked(), Coordinate(i, j));
	  if (cellDblClickMapper_)
	    cellDblClickMapper_->mapConnect(t->doubleClicked(),
					    Coordinate(i, j));
	}

	Utils::itoa(d.day(), buf);
	t->setText(WString::fromUTF8(buf));

	WDate date(d.year(), d.month(), d.day());

	std::string styleClass;

	if (d.month() != currentMonth_)
	  styleClass += " Wt-cal-oom";

	if (isSelected(date))
	  styleClass += " Wt-cal-sel";

	if (d == today) {
	  if (!isSelected(date))
	    styleClass += " Wt-cal-now";
	  t->setToolTip("Today");
	} else
	  t->setToolTip("");

	t->setStyleClass(styleClass.c_str());

	d += date_duration(1);
      }
    }

    needRenderMonth_ = false;
  }

  WCompositeWidget::render();
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

  if (rerender)
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
  if (!multipleSelection_ && singleClickSelect_) {
    cellDblClicked(weekday);
    return;
  }

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

      selectionChanged().emit();
      renderMonth();

    } else {
      selection_.clear();
      selection_.insert(d);

      selectionChanged().emit();
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
      selected().emit(WDate(dt.year(), dt.month(), dt.day()));
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
  } catch (boost::bad_lexical_cast& e) {
  }
}

}
