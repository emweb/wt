/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WCalendar.h"

#include "Wt/WComboBox.h"
#include "Wt/WInPlaceEdit.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/WTemplate.h"
#include "Wt/WText.h"

#include "WebUtils.h"

namespace Wt {

LOGGER("WCalendar");

// Because WDate returns days and weeks as WT_USTRING, we need this:
#ifndef WT_TARGET_JAVA
#define DATE_NAME_STR(e) e
#else
#define DATE_NAME_STR(e) WString::fromUTF8(e)
#endif

WCalendar::WCalendar()
{
  create();
  impl_->addStyleClass("Wt-calendar");
}

void WCalendar::setSelectionMode(SelectionMode mode) 
{
  if (selectionMode_ != mode) {
    if (mode != SelectionMode::Extended && selection_.size() > 1) {
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
  selectionMode_ = SelectionMode::Single;
  singleClickSelect_ = false;
  horizontalHeaderFormat_ = CalendarHeaderFormat::ShortDayNames;
  firstDayOfWeek_ = 1;

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

  std::unique_ptr<WTemplate> t(new WTemplate());
  impl_ = t.get();
  setImplementation(std::move(t));
  impl_->setTemplateText(WString::fromUTF8(text.str()),
			 TextFormat::UnsafeXHTML);
  impl_->setStyleClass("Wt-cal");

  setSelectable(false);

  std::unique_ptr<WText> prevMonth(new WText(tr("Wt.WCalendar.PrevMonth")));
  prevMonth->setStyleClass("Wt-cal-navbutton");
  prevMonth->clicked().connect(this, &WCalendar::browseToPreviousMonth);

  std::unique_ptr<WText> nextMonth(new WText(tr("Wt.WCalendar.NextMonth")));
  nextMonth->setStyleClass("Wt-cal-navbutton");
  nextMonth->clicked().connect(this, &WCalendar::browseToNextMonth);

  std::unique_ptr<WComboBox> monthEdit(new WComboBox());
  monthEdit_ = monthEdit.get();

  monthEdit->setInline(true);
  for (unsigned i = 0; i < 12; ++i)
    monthEdit->addItem(WDate::longMonthName(i+1));
  monthEdit->activated().connect(this, &WCalendar::monthChanged);
  monthEdit->setDisabled(!WApplication::instance()->environment().ajax());

  std::unique_ptr<WInPlaceEdit> yearEdit(new WInPlaceEdit(""));
  yearEdit_ = yearEdit.get();

  yearEdit->setButtonsEnabled(false);
  yearEdit->lineEdit()->setTextSize(4);
  yearEdit->setStyleClass("Wt-cal-year");
  yearEdit->valueChanged().connect(this, &WCalendar::yearChanged);

  impl_->bindWidget("nav-prev", std::move(prevMonth));
  impl_->bindWidget("nav-next", std::move(nextMonth));
  impl_->bindWidget("month", std::move(monthEdit));
  impl_->bindWidget("year", std::move(yearEdit));

  setHorizontalHeaderFormat(horizontalHeaderFormat_);
  setFirstDayOfWeek(firstDayOfWeek_);
}

void WCalendar::enableAjax()
{
  WCompositeWidget::enableAjax();
  monthEdit_->enable();

}

void WCalendar::load()
{
  WCompositeWidget::load();
  if(WApplication::instance()->environment().ajax()){
    monthEdit_->enable();
  }
}

void WCalendar::setFirstDayOfWeek(int dayOfWeek)
{
  firstDayOfWeek_ = dayOfWeek;

  for (unsigned i = 0; i < 7; ++i) {
    int day = (i + firstDayOfWeek_ - 1) % 7 + 1;

    WString title = WDate::longDayName(day);
    impl_->bindString("t" + std::to_string(i), 
		      title, 
		      TextFormat::UnsafeXHTML);

    WString abbr;
    switch (horizontalHeaderFormat_) {
    case CalendarHeaderFormat::SingleLetterDayNames:
      abbr = WString::fromUTF8(WDate::shortDayName(day).toUTF8().substr(0, 1));
      break;
    case CalendarHeaderFormat::ShortDayNames:
      abbr = WDate::shortDayName(day);
      break;
    case CalendarHeaderFormat::LongDayNames:
      abbr = WDate::longDayName(day);
      break;
    }
   
    impl_->bindString("d" + std::to_string(i), abbr,
		      TextFormat::UnsafeXHTML);
  }

  renderMonth();
}

void WCalendar::setHorizontalHeaderFormat(CalendarHeaderFormat format)
{
  std::string d;
  switch (format) {
  case CalendarHeaderFormat::SingleLetterDayNames:
    d = "d1"; break;
  case CalendarHeaderFormat::ShortDayNames:
    d = "d3"; break;
  case CalendarHeaderFormat::LongDayNames:
    d = "dlong"; break;
  default:
    LOG_ERROR("setHorizontalHeaderFormat(): "
	      "improper horizontal header format.");
    format = CalendarHeaderFormat::SingleLetterDayNames;
    d = "d1";
  }

  horizontalHeaderFormat_ = format;

  impl_->bindString("table-class", d, TextFormat::UnsafeXHTML);

  setFirstDayOfWeek(firstDayOfWeek_);
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
#ifndef WT_TARGET_JAVA
    char buf[30];
#else
    char *buf;
#endif // WT_TARGET_JAVA

    int m = currentMonth_ - 1;
    if (monthEdit_->currentIndex() != m)
      monthEdit_->setCurrentIndex(m);

    int y = currentYear_;
    Utils::itoa(y, buf);
    if (yearEdit_->text().toUTF8() != buf)
      yearEdit_->setText(WString::fromUTF8(buf));

    // The first line contains the last day of the previous month.
    WDate d(currentYear_, currentMonth_, 1);
    d = d.addDays(-1);
 
    d = WDate::previousWeekday(d, firstDayOfWeek_);

    for (unsigned i = 0; i < 6; ++i) {
      for (unsigned j = 0; j < 7; ++j) {
	Utils::itoa(i * 7 + j, buf);
	std::string cell = std::string("c") + buf;
	
	WDate date(d.year(), d.month(), d.day());

	WWidget *w = impl_->resolveWidget(cell);
	WWidget *rw = renderCell(w, date);
	WInteractWidget* iw = dynamic_cast<WInteractWidget*>(rw->webWidget());

	if (rw != w)
	  impl_->bindWidget(cell, std::unique_ptr<WWidget>(rw));

	if (iw && iw != w) {
	  if (clicked().isConnected()
	      || (selectionMode_ == SelectionMode::Extended)
	      || (selectionMode_ != SelectionMode::Extended && 
		  singleClickSelect_ && activated().isConnected())) {
            const Coordinate c(i, j);
	    iw->clicked().connect
	      (this,
	       std::bind(&WCalendar::cellClicked, this, c));
          }

	  if ((selectionMode_ != SelectionMode::Extended &&
	       !singleClickSelect_ && (activated().isConnected() ||
		   selectionChanged().isConnected()))) {
            const Coordinate c(i, j);
	    iw->doubleClicked().connect
	      (this,
	       std::bind(&WCalendar::cellDblClicked, this, c));
          }
	}

    d = d.addDays(1);
      }
    }

    needRenderMonth_ = false;
  }

  WCompositeWidget::render(flags);
}

WWidget *WCalendar::renderCell(WWidget* widget, const WDate& date)
{
  WText* t = dynamic_cast<WText*>(widget);

  if (!t) {
    t = new WText();
    t->setInline(false);
    t->setTextFormat(TextFormat::Plain);
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
    t->setToolTip(WString::tr("Wt.WCalendar.today"));
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
  if (selectionMode_ == SelectionMode::Extended) {
    selection_ = dates;
    renderMonth();
  } else if (selectionMode_ == SelectionMode::Single) {
    if (dates.empty())
      clearSelection();
    else
      select(*dates.begin());
  }
}

void WCalendar::selectInCurrentMonth(const WDate& d)
{
  if (d.month() == currentMonth_ &&
      selectionMode_ != SelectionMode::None) {
    if (selectionMode_ == SelectionMode::Extended) {
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
  WDate dt = dateForCell(weekday.i, weekday.j);
  if (isInvalid(dt))
    return;

  selectInCurrentMonth(dt);
  clicked().emit(dt);
  
  if (selectionMode_ != SelectionMode::Extended && 
      singleClickSelect_)
    activated().emit(dt);
}

void WCalendar::cellDblClicked(Coordinate weekday)
{
  WDate dt = dateForCell(weekday.i, weekday.j);
  if (isInvalid(dt))
    return;

  selectInCurrentMonth(dt);

  if (selectionMode_ != SelectionMode::Extended &&
      !singleClickSelect_)
    activated().emit(dt);
}

WDate WCalendar::dateForCell(int week, int dayOfWeek)
{
  WDate d(currentYear_, currentMonth_, 1);
  d = d.addDays(-1);
  d = WDate::previousWeekday(d, firstDayOfWeek_);
  d = d.addDays(week * 7 + dayOfWeek);
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
    int year = Utils::stoi(yearStr.toUTF8());

    if (currentYear_ != year &&
	(year >= 1900 && year <= 2200)) { // ??
      currentYear_ = year;

      emitCurrentPageChanged();
      renderMonth();
    }
  } catch (std::exception& e) {
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
