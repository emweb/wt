/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WDatePicker"

#include "Wt/WApplication"
#include "Wt/WCalendar"
#include "Wt/WContainerWidget"
#include "Wt/WDateValidator"
#include "Wt/WImage"
#include "Wt/WInteractWidget"
#include "Wt/WTemplate"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"

#include "Utils.h"

namespace Wt {

WDatePicker::WDatePicker(WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  createDefault();
}

WDatePicker::WDatePicker(WInteractWidget *displayWidget,
			 WLineEdit *forEdit, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  create(displayWidget, forEdit);
}

WDatePicker::~WDatePicker()
{
  WApplication::instance()->doJavaScript
    (WT_CLASS ".remove('" + popup_->id() + "');");
}

void WDatePicker::createDefault()
{
  WImage *icon = new WImage(WApplication::resourcesUrl() + "calendar_edit.png");
  icon->setVerticalAlignment(AlignMiddle);
  WLineEdit *lineEdit = new WLineEdit();

  create(icon, lineEdit);

  layout_->insertWidget(0, lineEdit);

  lineEdit->setValidator(new WDateValidator(format_, this));
}

void WDatePicker::create(WInteractWidget *displayWidget,
			 WLineEdit *forEdit)
{
  setImplementation(layout_ = new WContainerWidget());

  displayWidget_ = displayWidget;
  forEdit_ = forEdit;
  forEdit_->setVerticalAlignment(AlignMiddle);
  forEdit_->changed().connect(this, &WDatePicker::setFromLineEdit);
  format_ = "dd/MM/yyyy";

  layout_->setInline(true);
  layout_->addWidget(displayWidget);

  const char *TEMPLATE =
    "${shadow-x1-x2}"
    "${calendar}"
    "<div style=\"text-align:center; margin-top:3px\">${close}</div>";

  layout_->addWidget(popup_ = new WTemplate(WString::fromUTF8(TEMPLATE)));

  calendar_ = new WCalendar();
  calendar_->activated().connect(popup_, &WWidget::hide);
  calendar_->selectionChanged().connect(this, &WDatePicker::setFromCalendar);

  WPushButton *closeButton = new WPushButton(tr("Wt.WDatePicker.Close"));
  closeButton->clicked().connect(popup_, &WWidget::hide);

  popup_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  popup_->bindWidget("calendar", calendar_);
  popup_->bindWidget("close", closeButton);

  popup_->hide();
  popup_->setPopup(true);
  popup_->setPositionScheme(Absolute);
  popup_->setStyleClass("Wt-outset Wt-datepicker");

  // This confuses the close button hide ? XXX
  //WApplication::instance()->globalEscapePressed()
  //  .connect(popup_, &WWidget::hide);
  popup_->escapePressed().connect(popup_, &WWidget::hide);
  displayWidget->clicked().connect(popup_, &WWidget::show);
  displayWidget->clicked().connect(positionJS_);
  displayWidget->clicked().connect(this, &WDatePicker::setFromLineEdit);

  setGlobalPopup(false);
}

void WDatePicker::setPopupVisible(bool visible)
{
  popup_->setHidden(!visible);
}

void WDatePicker::setGlobalPopup(bool global)
{
  positionJS_.setJavaScript("function() { " WT_CLASS ".positionAtWidget('"
			    + popup_->id()  + "','" + displayWidget_->id()
			    + "', " WT_CLASS ".Horizontal, "
			    + (global ? "true" : "false") + ");}");
}

void WDatePicker::setFormat(const WT_USTRING& format)
{
  format_ = format;

  WDateValidator *dv = dynamic_cast<WDateValidator *>(forEdit_->validator());
  if (dv)
    dv->setFormat(format);
}

void WDatePicker::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    const WDate& calDate = *calendar_->selection().begin();

    forEdit_->setText(calDate.toString(format_));
    forEdit_->changed().emit();
  }
}

WDate WDatePicker::date() const
{
  return WDate::fromString(forEdit_->text(), format_);
}

void WDatePicker::setDate(const WDate& date)
{
  if (!date.isNull()) {
    forEdit_->setText(date.toString(format_));
    calendar_->select(date);
    calendar_->browseTo(date);
  }
}

void WDatePicker::setFromLineEdit()
{
  WDate d = WDate::fromString(forEdit_->text(), format_);

  if (d.isValid()) {
    if (calendar_->selection().empty()) {
      calendar_->select(d);
      calendar_->selectionChanged().emit();
    } else {
      WDate j = Utils::first(calendar_->selection());
      if (j != d) {
	calendar_->select(d);
	calendar_->selectionChanged().emit();
      }
    }

    calendar_->browseTo(d);
  }
}

Signal<>& WDatePicker::changed()
{
  return calendar_->selectionChanged();
}

void WDatePicker::setEnabled(bool enabled)
{
  setDisabled(!enabled);
}

void WDatePicker::setDisabled(bool disabled)
{
  forEdit_->setDisabled(disabled);
  displayWidget_->setHidden(disabled);
}

void WDatePicker::setHidden(bool hidden)
{
  WCompositeWidget::setHidden(hidden);
  forEdit_->setHidden(hidden);
  displayWidget_->setHidden(hidden);
}

void WDatePicker::setBottom(const WDate& bottom)
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(forEdit_->validator());
  if (dv) {
    dv->setBottom(bottom);
    calendar_->setBottom(bottom);
  }
}

WDate WDatePicker::bottom() const
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(forEdit_->validator());
  if (dv)
    return dv->bottom();
  else 
    return WDate();
}
  
void WDatePicker::setTop(const WDate& top) 
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(forEdit_->validator());
  if (dv) {
    dv->setTop(top);
    calendar_->setTop(top);
  }
}

WDate WDatePicker::top() const
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(forEdit_->validator());
  if (dv)
    return dv->top();
  else 
    return WDate();
}
}
