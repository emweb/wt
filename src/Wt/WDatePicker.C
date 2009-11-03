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
#include "Wt/WLineEdit"
#include "Wt/WPushButton"

namespace Wt {

WDatePicker::WDatePicker(WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  createDefault(false);
}

WDatePicker::WDatePicker(bool i18n, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  createDefault(i18n);
}

WDatePicker::WDatePicker(WInteractWidget *displayWidget,
			 WLineEdit *forEdit,
			 bool i18n, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  create(displayWidget, forEdit, i18n);
}

void WDatePicker::createDefault(bool i18n)
{
  WImage *icon = new WImage(WApplication::resourcesUrl() + "calendar_edit.png");
  WLineEdit *lineEdit = new WLineEdit();

  create(icon, lineEdit, i18n);

  layout_->insertWidget(0, lineEdit);

  lineEdit->setValidator(new WDateValidator(format_, this));
}

void WDatePicker::create(WInteractWidget *displayWidget,
			 WLineEdit *forEdit, bool i18n)
{
  setImplementation(layout_ = new WContainerWidget());

  displayWidget_ = displayWidget;
  forEdit_ = forEdit;
  format_ = "dd/MM/yyyy";

  const char *CSS_RULES_NAME = "Wt::WDatePicker";

  WApplication *app = WApplication::instance();
  if (!app->styleSheet().isDefined(CSS_RULES_NAME))
    app->styleSheet().addRule(".Wt-popup",
			      "background-color: #EEEEEE;"
			      "border: 1px solid #000000;"
			      "padding: 2px;", CSS_RULES_NAME);

  layout_->setInline(true);
  layout_->addWidget(displayWidget);
  layout_->addWidget(popup_ = new WContainerWidget());

  calendar_ = new WCalendar(i18n, popup_);
  calendar_->selected().connect(SLOT(popup_, WWidget::hide));
  calendar_->selectionChanged()
    .connect(SLOT(this, WDatePicker::setFromCalendar));

  WContainerWidget *buttonContainer = new WContainerWidget(popup_);
  buttonContainer->setContentAlignment(AlignCenter);
  WPushButton *closeButton
    = new WPushButton(i18n ? tr("Close") : "Close", buttonContainer);
  closeButton->clicked().connect(SLOT(popup_, WWidget::hide));

  popup_->hide();
  popup_->setPopup(true);
  popup_->setPositionScheme(Absolute);
  popup_->setStyleClass("Wt-popup");

  popup_->escapePressed().connect(SLOT(popup_, WWidget::hide));
  displayWidget->clicked().connect(SLOT(popup_, WWidget::show));

  positionJS_.setJavaScript("function() { " WT_CLASS ".positionAtWidget('"
			    + popup_->id()  + "','" + displayWidget->id()
			    + "');}");
  displayWidget->clicked().connect(positionJS_);
  displayWidget->clicked().connect(SLOT(this, WDatePicker::setFromLineEdit));
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
    calendar_->select(d);
    calendar_->browseTo(d);
  }
}

void WDatePicker::setEnabled(bool enabled)
{
  setDisabled(false);
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

}
