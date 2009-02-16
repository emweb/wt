/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WDatePicker"

#include "Wt/WApplication"
#include "Wt/WDateValidator"
#include "Wt/WInteractWidget"
#include "Wt/WContainerWidget"
#include "Wt/WCalendar"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"

namespace Wt {

WDatePicker::WDatePicker(WInteractWidget *displayWidget,
			 WLineEdit *forEdit,
			 bool i18n, WContainerWidget *parent)
  : WCompositeWidget(parent),
    format_("dd/MM/yyyy"),
    displayWidget_(displayWidget),
    forEdit_(forEdit)
{
  setImplementation(layout_ = new WContainerWidget());

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
  calendar_->selected.connect(SLOT(popup_, WWidget::hide));
  calendar_->selectionChanged.connect(SLOT(this, WDatePicker::setFromCalendar));

  WContainerWidget *buttonContainer = new WContainerWidget(popup_);
  buttonContainer->setContentAlignment(AlignCenter);
  WPushButton *closeButton
    = new WPushButton(i18n ? tr("Close") : "Close", buttonContainer);
  closeButton->clicked.connect(SLOT(popup_, WWidget::hide));

  popup_->hide();
  popup_->setPopup(true);
  popup_->setPositionScheme(Absolute);
  popup_->setStyleClass("Wt-popup");

  popup_->escapePressed.connect(SLOT(popup_, WWidget::hide));
  displayWidget->clicked.connect(SLOT(popup_, WWidget::show));
  displayWidget->clicked.connect(SLOT(this, WDatePicker::setFromLineEdit));
}

void WDatePicker::setFormat(const WString& format)
{
  format_ = format;
}

void WDatePicker::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    const WDate& calDate = *calendar_->selection().begin();

    forEdit_->setText(calDate.toString(format_));
    forEdit_->changed.emit();
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

}
