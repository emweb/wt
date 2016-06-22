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
#include "Wt/WLogger"
#include "Wt/WPopupWidget"
#include "Wt/WPushButton"
#include "Wt/WTemplate"
#include "Wt/WTheme"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WDateEdit.min.js"
#endif

namespace Wt {

LOGGER("WDateEdit");

WDateEdit::WDateEdit(WContainerWidget *parent)
  : WLineEdit(parent),
    popup_(0)
{
  changed().connect(this, &WDateEdit::setFromLineEdit);

  calendar_ = new WCalendar();
  calendar_->setSingleClickSelect(true);
  calendar_->activated().connect(this, &WWidget::setFocus);
  calendar_->selectionChanged().connect(this, &WDateEdit::setFromCalendar);

  setValidator(new WDateValidator("dd/MM/yyyy", this));
}

WDateEdit::~WDateEdit()
{
  if (!popup_) {
    // calendar_ is not owned by popup_, because it doesn't exist
    delete calendar_;
  }
}

void WDateEdit::load()
{
  bool wasLoaded = loaded();

  WLineEdit::load();
  // Loading of popup_ is deferred (see issue #4897)

  if (wasLoaded)
    return;

  const char *TEMPLATE = "${calendar}";

  WTemplate *t = new WTemplate(WString::fromUTF8(TEMPLATE));
  popup_ = new WPopupWidget(t, this);
  if (isHidden()) {
    popup_->setHidden(true);
  }
  popup_->setAnchorWidget(this);
  popup_->setTransient(true);

  calendar_->activated().connect(popup_, &WPopupWidget::hide);
  t->bindWidget("calendar", calendar_);

  WApplication::instance()->theme()->apply(this, popup_, DatePickerPopupRole);

  escapePressed().connect(popup_, &WPopupWidget::hide);
  escapePressed().connect(this, &WWidget::setFocus);
}

WDateValidator *WDateEdit::validator() const
{
  return dynamic_cast<WDateValidator *>(WLineEdit::validator());
}

void WDateEdit::setFormat(const WT_USTRING& format)
{
  WDateValidator *dv = validator();

  if (dv) {
    WDate d = this->date();
    dv->setFormat(format);
    setDate(d);
  } else {
    LOG_WARN("setFormat() ignored since validator is not a WDateValidator");
  }
}

WT_USTRING WDateEdit::format() const
{
  WDateValidator *dv = validator();

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
  WDateValidator *dv = validator();
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
  WDateValidator *dv = validator();
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
    """var o = jQuery.data(" + jsRef() + ", 'dobj');"
    """if (o) o." + methodName + "(dobj, event);"
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
  if (flags & RenderFull) {
    defineJavaScript();
    WDateValidator *dv = validator();
    if (dv) {
      setTop(dv->top());
      setBottom(dv->bottom());
      setFormat(dv->format());
    }
  }

  WLineEdit::render(flags);
}

}
