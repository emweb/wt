/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDateEdit"

#include "Wt/WApplication"
#include "Wt/WCalendar"
#include "Wt/WContainerWidget"
#include "Wt/WDateValidator"
#include "Wt/WLineEdit"
#include "Wt/WPopupWidget"
#include "Wt/WPushButton"
#include "Wt/WTemplate"
#include "Wt/WTheme"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WDateEdit.min.js"
#endif

namespace Wt {

WDateEdit::WDateEdit(WContainerWidget *parent)
  : WLineEdit(parent)
{
  changed().connect(this, &WDateEdit::setFromLineEdit);

  const char *TEMPLATE = "${calendar}";

  WTemplate *t = new WTemplate(WString::fromUTF8(TEMPLATE));
  popup_ = new WPopupWidget(t, this);
  popup_->setAnchorWidget(this);
  popup_->setTransient(true);

  calendar_ = new WCalendar();
  calendar_->activated().connect(popup_, &WPopupWidget::hide);
  calendar_->activated().connect(this, &WDateEdit::setFocus);
  calendar_->selectionChanged().connect(this, &WDateEdit::setFromCalendar);

  t->bindWidget("calendar", calendar_);

  WApplication::instance()->theme()->apply(this, popup_, DatePickerPopupRole);

  t->escapePressed().connect(popup_, &WTemplate::hide);
  t->escapePressed().connect(this, &WDateEdit::setFocus);

  setValidator(new WDateValidator("dd/MM/yyyy", this));
}

WDateValidator *WDateEdit::validator() const
{
  return dynamic_cast<WDateValidator *>(WLineEdit::validator());
}

void WDateEdit::setFormat(const WT_USTRING& format)
{
  WDate d = this->date();
  validator()->setFormat(format);
  setDate(d);
}

WT_USTRING WDateEdit::format() const
{
  return validator()->format();
}

void WDateEdit::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    WDate calDate = Utils::first(calendar_->selection());
    setText(calDate.toString(format()));
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
  popup_->setHidden(hidden, animation);
}

void WDateEdit::setBottom(const WDate& bottom)
{
  validator()->setBottom(bottom);
  calendar_->setBottom(bottom);
}

WDate WDateEdit::bottom() const
{
  return validator()->bottom();
}
  
void WDateEdit::setTop(const WDate& top) 
{
  validator()->setTop(top);
  calendar_->setTop(top);
}

WDate WDateEdit::top() const
{
  return validator()->top();
}

void WDateEdit::connectJavaScript(Wt::EventSignalBase& s,
				  const std::string& methodName)
{
  std::string jsFunction = 
    "function(obj, event) {"
    """var o = jQuery.data(" + jsRef() + ", 'obj');"
    """if (o) o." + methodName + "(obj, event);"
    "}";

  s.connect(jsFunction);
}

void WDateEdit::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WDateEdit.js", "WDateEdit", wtjs1);

  std::string jsObj = "new " WT_CLASS ".WDateEdit("
    + app->javaScriptClass() + "," + jsRef() + "," 
    + popup_->jsRef() + ");";

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
  if (flags & RenderFull) {
    defineJavaScript();
    setTop(validator()->top());
    setBottom(validator()->bottom());
    setFormat(validator()->format());
  }

  WLineEdit::render(flags);
}

}
