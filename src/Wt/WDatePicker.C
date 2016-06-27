/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDatePicker"

#include "Wt/WApplication"
#include "Wt/WCalendar"
#include "Wt/WContainerWidget"
#include "Wt/WDateValidator"
#include "Wt/WImage"
#include "Wt/WInteractWidget"
#include "Wt/WPopupWidget"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WTemplate"
#include "Wt/WTheme"

#include "WebUtils.h"

namespace Wt {

WDatePicker::WDatePicker(WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  createDefault(0);
}

WDatePicker::WDatePicker(WInteractWidget *displayWidget,
			 WLineEdit *forEdit, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  create(displayWidget, forEdit);
}

WDatePicker::WDatePicker(WLineEdit *forEdit, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  createDefault(forEdit);
}

WDatePicker::~WDatePicker()
{
  WApplication::instance()->doJavaScript
    (WT_CLASS ".remove('" + popup_->id() + "');");
}

void WDatePicker::createDefault(WLineEdit *forEdit)
{
  WImage *icon = new WImage(WApplication::relativeResourcesUrl() 
			    + "calendar_edit.png");
  icon->resize(16, 16);
  icon->setVerticalAlignment(AlignMiddle);

  if (!forEdit) {
    forEdit = new WLineEdit();
    create(icon, forEdit);
    layout_->insertWidget(0, forEdit);
  } else
    create(icon, forEdit);
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
  layout_->setAttributeValue("style", "white-space: nowrap");

  const char *TEMPLATE = "${calendar}";

  WTemplate *t = new WTemplate(WString::fromUTF8(TEMPLATE));
  popup_ = new WPopupWidget(t, this);
  popup_->setAnchorWidget(displayWidget_, Horizontal);
  popup_->setTransient(true);

  calendar_ = new WCalendar();
  calendar_->setSingleClickSelect(true);
  calendar_->activated().connect(popup_, &WWidget::hide);
  calendar_->activated().connect(this, &WDatePicker::onPopupHidden);
  calendar_->selectionChanged().connect(this, &WDatePicker::setFromCalendar);

  t->escapePressed().connect(popup_, &WTemplate::hide);
  t->escapePressed().connect(forEdit_, &WWidget::setFocus);

  t->bindWidget("calendar", calendar_);

  WApplication::instance()->theme()->apply(this, popup_, DatePickerPopupRole);

  displayWidget->clicked().connect(popup_, &WWidget::show);
  displayWidget->clicked().connect(this, &WDatePicker::setFromLineEdit);

  if (!forEdit_->validator())
    forEdit_->setValidator(new WDateValidator(format_, this));
}

void WDatePicker::setPopupVisible(bool visible)
{
  popup_->setHidden(!visible);
}

void WDatePicker::onPopupHidden()
{
  forEdit_->setFocus(true);
  popupClosed();
}

void WDatePicker::setGlobalPopup(bool global)
{ 
  popup_->toggleStyleClass("wt-no-reparent", global);
}

void WDatePicker::setFormat(const WT_USTRING& format)
{
  WDate d = this->date();

  format_ = format;

  WDateValidator *dv = dynamic_cast<WDateValidator *>(forEdit_->validator());
  if (dv)
    dv->setFormat(format);

  setDate(d);
}

void WDatePicker::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    const WDate& calDate = *calendar_->selection().begin();

    forEdit_->setText(calDate.toString(format_));
    forEdit_->changed().emit();
  }

  changed_.emit();
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

void WDatePicker::setEnabled(bool enabled)
{
  setDisabled(!enabled);
}

void WDatePicker::setDisabled(bool disabled)
{
  WCompositeWidget::setDisabled(disabled);

  forEdit_->setDisabled(disabled);
  displayWidget_->setHidden(disabled);
}

void WDatePicker::setHidden(bool hidden, const WAnimation& animation)
{
  WCompositeWidget::setHidden(hidden, animation);
  forEdit_->setHidden(hidden, animation);
  displayWidget_->setHidden(hidden, animation);
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

void WDatePicker::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull) {
    WDateValidator *dv = dynamic_cast<WDateValidator *>(forEdit_->validator());

    if (dv) {
      setTop(dv->top());
      setBottom(dv->bottom());
      setFormat(dv->format());
    }
  }

  WCompositeWidget::render(flags);
}

}
