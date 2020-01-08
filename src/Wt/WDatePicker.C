/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDatePicker.h"

#include "Wt/WApplication.h"
#include "Wt/WCalendar.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WDateValidator.h"
#include "Wt/WImage.h"
#include "Wt/WInteractWidget.h"
#include "Wt/WPopupWidget.h"
#include "Wt/WLineEdit.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"
#include "Wt/WTheme.h"

#include "WebUtils.h"

namespace Wt {

WDatePicker::WDatePicker()
{
  createDefault(nullptr);
}

WDatePicker::WDatePicker(std::unique_ptr<WInteractWidget> displayWidget,
			 WLineEdit *forEdit)
{
  create(std::move(displayWidget), forEdit);
}

WDatePicker::WDatePicker(WLineEdit *forEdit)
{
  createDefault(forEdit);
}

WDatePicker::~WDatePicker()
{ }

void WDatePicker::createDefault(WLineEdit *forEdit)
{
  std::unique_ptr<WImage> icon
    (new WImage(WApplication::relativeResourcesUrl() + "date.gif"));
  icon->resize(16, 16);
  icon->setVerticalAlignment(AlignmentFlag::Middle);

  if (!forEdit) {
    std::unique_ptr<WLineEdit> edit(new WLineEdit());
    create(std::move(icon), edit.get());
    layout_->insertWidget(0, std::move(edit));
  } else
    create(std::move(icon), forEdit);
}

void WDatePicker::create(std::unique_ptr<WInteractWidget> displayWidget,
			 WLineEdit *forEdit)
{
  layout_ = new WContainerWidget();
  setImplementation(std::unique_ptr<WContainerWidget>(layout_));

  displayWidget_ = displayWidget.get();
  forEdit_ = forEdit;
  forEdit_->setVerticalAlignment(AlignmentFlag::Middle);
  forEdit_->changed().connect(this, &WDatePicker::setFromLineEdit);

  format_ = "dd/MM/yyyy";

  layout_->setInline(true);
  layout_->addWidget(std::move(displayWidget));
  layout_->setAttributeValue("style", "white-space: nowrap");

  calendar_ = new WCalendar();
  calendar_->setSingleClickSelect(true);
  calendar_->activated().connect(this, &WDatePicker::onPopupHidden);
  calendar_->selectionChanged().connect(this, &WDatePicker::setFromCalendar);

  const char *TEMPLATE = "${calendar}";

  WTemplate *temp;
  std::unique_ptr<WTemplate> t(temp = new WTemplate(WString::fromUTF8(TEMPLATE)));
  popup_.reset(new WPopupWidget(std::move(t)));
  temp->escapePressed().connect(popup_.get(), &WTemplate::hide);
  temp->escapePressed().connect(forEdit_, &WWidget::setFocus);
  temp->bindWidget("calendar", std::unique_ptr<WWidget>(calendar_));

  popup_->setAnchorWidget(displayWidget_, Orientation::Horizontal);
  popup_->setTransient(true);
  calendar_->activated().connect(popup_.get(), &WWidget::hide);

  WApplication::instance()->theme()->apply(this, popup_.get(), DatePickerPopup);

  displayWidget_->clicked().connect(popup_.get(), &WWidget::show);
  displayWidget_->clicked().connect(this, &WDatePicker::setFromLineEdit);

  if (!forEdit_->validator())
    forEdit_->setValidator(std::make_shared<WDateValidator>(format_));
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

void WDatePicker::setFormat(const WT_USTRING& format)
{
  WDate d = this->date();

  format_ = format;

  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv)
    dv->setFormat(format);

  setDate(d);
}

void WDatePicker::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    const WDate& calDate = *calendar_->selection().begin();

    forEdit_->setText(calDate.toString(format_));
    forEdit_->textInput().emit();
    forEdit_->changed().emit();
  }

  changed_.emit();
}

WDate WDatePicker::date() const
{
  return WDate::fromString(forEdit_->text(), format_);
}

std::shared_ptr<WDateValidator> WDatePicker::dateValidator() const
{
  return std::dynamic_pointer_cast<WDateValidator>(forEdit_->validator());
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
  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv) {
    dv->setBottom(bottom);
    calendar_->setBottom(bottom);
  }
}

WDate WDatePicker::bottom() const
{
  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv)
    return dv->bottom();
  else 
    return WDate();
}
  
void WDatePicker::setTop(const WDate& top) 
{
  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv) {
    dv->setTop(top);
    calendar_->setTop(top);
  }
}

WDate WDatePicker::top() const
{
  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv)
    return dv->top();
  else 
    return WDate();
}

void WDatePicker::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full)) {
    std::shared_ptr<WDateValidator> dv = dateValidator();

    if (dv) {
      setTop(dv->top());
      setBottom(dv->bottom());
      setFormat(dv->format());
    }
  }

  WCompositeWidget::render(flags);
}

}
