/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDateEdit.h"

#include "Wt/WApplication.h"
#include "Wt/WCalendar.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WDateValidator.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WPopupWidget.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"
#include "Wt/WTheme.h"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WDateEdit.min.js"
#endif

namespace Wt {

LOGGER("WDateEdit");

WDateEdit::WDateEdit()
  : customFormat_(false)
{
  changed().connect(this, &WDateEdit::setFromLineEdit);

  uCalendar_ = cpp14::make_unique<WCalendar>();
  calendar_ = uCalendar_.get();
  calendar_->setSingleClickSelect(true);
  calendar_->activated().connect(this, &WDateEdit::setFocusTrue);
  calendar_->selectionChanged().connect(this, &WDateEdit::setFromCalendar);

  setValidator(std::make_shared<WDateValidator>(WApplication::instance()->locale().dateFormat()));
}

WDateEdit::~WDateEdit()
{ }

void WDateEdit::load()
{
  bool wasLoaded = loaded();

  WLineEdit::load();
  // Loading of popup_ is deferred (see issue #4897)

  if (wasLoaded)
    return;

  const char *TEMPLATE = "${calendar}";
  std::unique_ptr<WTemplate> t(new WTemplate(WString::fromUTF8(TEMPLATE)));
  WTemplate *temp = t.get();

  popup_.reset(new WPopupWidget(std::move(t)));
  if (isHidden()) {
    popup_->setHidden(true);
  }
  popup_->setAnchorWidget(this);
  popup_->setTransient(true);

  calendar_->activated().connect(popup_.get(), &WPopupWidget::hide);
  temp->bindWidget("calendar", std::move(uCalendar_));

  WApplication::instance()->theme()->apply
    (this, popup_.get(), DatePickerPopup);

  escapePressed().connect(popup_.get(), &WPopupWidget::hide);
  escapePressed().connect(this, &WDateEdit::setFocusTrue);
}

void WDateEdit::refresh()
{
  WLineEdit::refresh();

  auto dv = dateValidator();

  if (!customFormat_ && dv) {
    WDate d = this->date();
    dv->setFormat(Wt::WApplication::instance()->locale().dateFormat());
    setDate(d);
  } else {
    LOG_WARN("setFormat() ignored since validator is not a WDateValidator");
  }
}

std::shared_ptr<WDateValidator> WDateEdit::dateValidator() const
{
  return std::dynamic_pointer_cast<WDateValidator>(WLineEdit::validator());
}

void WDateEdit::setFormat(const WT_USTRING& format)
{
  std::shared_ptr<WDateValidator> dv = dateValidator();

  if (dv) {
    WDate d = this->date();
    dv->setFormat(format);
    setDate(d);
    customFormat_ = true;
  } else {
    LOG_WARN("setFormat() ignored since validator is not a WDateValidator");
  }
}

WT_USTRING WDateEdit::format() const
{
  std::shared_ptr<WDateValidator> dv = dateValidator();

  if (dv) {
    return dv->format();
  } else {
    LOG_WARN("format() is bogus  since validator is not a WDateValidator");
    return WT_USTRING();
  }
}

void WDateEdit::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    WDate calDate = Utils::first(calendar_->selection());
    setText(calDate.toString(format()));
    textInput().emit();
    changed().emit();
  }
}

WDate WDateEdit::date() const
{
  return WDate::fromString(text(), format());
}

void WDateEdit::setDate(const WDate& date)
{
  if (!date.isNull()) {
    setText(date.toString(format()));
    calendar_->select(date);
    calendar_->browseTo(date);
  }
}

void WDateEdit::setFromLineEdit()
{
  WDate d = WDate::fromString(text(), format());

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

void WDateEdit::propagateSetEnabled(bool enabled)
{
  WLineEdit::propagateSetEnabled(enabled);
}

void WDateEdit::setHidden(bool hidden, const WAnimation& animation)
{
  WLineEdit::setHidden(hidden, animation);
  // rationale: when calling hide(), line edit and popup should go away. When
  // calling show(), line edit becomes visible, but popup is only shown when
  // using the widget.
  if (popup_ && hidden)
    popup_->setHidden(hidden, animation);
}

void WDateEdit::setBottom(const WDate& bottom)
{
  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv)
    dv->setBottom(bottom);

  calendar_->setBottom(bottom);
}

WDate WDateEdit::bottom() const
{
  return calendar_->bottom();
}
  
void WDateEdit::setTop(const WDate& top) 
{
  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv)
    dv->setTop(top);

  calendar_->setTop(top);
}

WDate WDateEdit::top() const
{
  return calendar_->top();
}

void WDateEdit::connectJavaScript(Wt::EventSignalBase& s,
				  const std::string& methodName)
{
  std::string jsFunction = 
    "function(dobj, event) {"
    """var o = " + jsRef() + ";"
    """if (o && o.wtDObj) o.wtDObj." + methodName + "(dobj, event);"
    "}";

  s.connect(jsFunction);
}

void WDateEdit::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WDateEdit.js", "WDateEdit", wtjs1);

  std::string jsObj = "new " WT_CLASS ".WDateEdit("
    + app->javaScriptClass() + "," + jsRef() + "," 
    + jsStringLiteral(popup_->id()) + ");";

  setJavaScriptMember(" WDateEdit", jsObj);

#ifdef WT_CNOR
  EventSignalBase& b = mouseMoved();
  EventSignalBase& c = keyWentDown();
#endif

  connectJavaScript(mouseMoved(), "mouseMove");
  connectJavaScript(mouseWentUp(), "mouseUp");
  connectJavaScript(mouseWentDown(), "mouseDown");
  connectJavaScript(mouseWentOut(), "mouseOut");
}

void WDateEdit::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full)) {
    defineJavaScript();
    std::shared_ptr<WDateValidator> dv = dateValidator();
    if (dv) {
      setTop(dv->top());
      setBottom(dv->bottom());
    }
  }

  WLineEdit::render(flags);
}

void WDateEdit::setFocusTrue()
{
  setFocus(true);
}

}
