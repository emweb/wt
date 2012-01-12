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
#include "Wt/WEnvironment"
#include "Wt/WPushButton"
#include "Wt/WTemplate"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WDatePicker.min.js"
#endif

namespace Wt {

WDatePicker::WDatePicker(WContainerWidget *parent)
  : WLineEdit(parent),
    preferNative_(false),
    popup_(0),
    calendar_(0),
    global_(false),
    jsPopup_(this, "popup")   
{
  setJavaScriptMember("_a", "0");

  WDateValidator *v = new WDateValidator("dd/MM/yyyy");
  setValidator(v);
}

WDatePicker::~WDatePicker()
{
  delete popup_;
}

void WDatePicker::setNativeControl(bool nativeControl)
{
  preferNative_ = nativeControl;
}

bool WDatePicker::nativeControl() const
{
  if (preferNative_) {
    const WEnvironment& env = WApplication::instance()->environment();
    if (env.agentIsOpera() && env.agent() >= WEnvironment::Opera10)
      return true;
  }

  return false;
}

void WDatePicker::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull) {
    bool useNative = nativeControl();

    setup(useNative);
  }

  WLineEdit::render(flags);
}

void WDatePicker::setText(const WT_USTRING& text)
{
  WLineEdit::setText(text);

  setFromLineEdit();
}

void WDatePicker::setFormData(const FormData& formData)
{
  WLineEdit::setFormData(formData);
}

void WDatePicker::updateDom(DomElement& element, bool all)
{
  WLineEdit::updateDom(element, all);

  if (all) {
    if (nativeControl())
      element.setAttribute("type", "date");
  }
}

void WDatePicker::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WDatePicker.js", "WDatePicker", wtjs1);

  std::string jsObj = "new " WT_CLASS ".WDatePicker("
    + app->javaScriptClass() + "," + jsRef() + ",'"
    + popup_->id() + "'," + (global_ ? "true" : "false") + ");";

  setJavaScriptMember("_a", "0;" + jsObj);
}

void WDatePicker::connectJavaScript(Wt::EventSignalBase& s,
				    const std::string& methodName)
{
  std::string jsFunction = 
    "function(obj, event) {"
    """var o = jQuery.data(" + jsRef() + ", 'obj');"
    """if (o) o." + methodName + "(obj, event);"
    "}";

  s.connect(jsFunction);
}

void WDatePicker::setup(bool useNative)
{
  if (useNative) {
  } else {
    addStyleClass("Wt-datepicker-edit");

    const char *TEMPLATE =
      "${shadow-x1-x2}"
      "${calendar}"
      "<div style=\"text-align:center; margin-top:3px\">${close}</div>";

    popup_ = new WTemplate(WString::fromUTF8(TEMPLATE),
			   WApplication::instance()->domRoot());

    calendar_ = new WCalendar();
    calendar_->selectionChanged().connect(this, &WDatePicker::setFromCalendar);

    WPushButton *closeButton = new WPushButton(tr("Wt.WDatePicker.Close"));

    popup_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
    popup_->bindWidget("calendar", calendar_);
    popup_->bindWidget("close", closeButton);

    popup_->hide();
    popup_->setPopup(true);
    popup_->setPositionScheme(Absolute);
    popup_->setStyleClass("Wt-outset Wt-datepicker");

    escapePressed().connect(popup_, &WWidget::hide);
    escapePressed().connect(this, &WDatePicker::done);

    calendar_->activated().connect(popup_, &WWidget::hide);
    calendar_->activated().connect(this, &WDatePicker::done);

    closeButton->clicked().connect(popup_, &WWidget::hide);
    closeButton->clicked().connect(this, &WDatePicker::done);

    defineJavaScript();

#ifdef WT_CNOR
    EventSignalBase& b = mouseMoved();
#endif

    connectJavaScript(mouseMoved(), "mouseMove");
    connectJavaScript(clicked(), "mouseClick");

    changed().connect(this, &WDatePicker::setFromLineEdit);
    jsPopup_.connect(this, &WDatePicker::onPopup);
  }
}

void WDatePicker::setPopupVisible(bool visible)
{
  if (visible) {
    setFromLineEdit();
    doJavaScript("jQuery.data(" + jsRef() + ", 'obj').showPopup()");
  } else {
    WApplication *app = WApplication::instance();
    app->root()->clicked().disconnect(globalClickConnection_);
    popup_->hide();
  }
}

void WDatePicker::onPopup()
{
  popup_->show();
  setFromLineEdit();

  if (!globalClickConnection_.connected()) {
    WApplication *app = WApplication::instance();
    globalClickConnection_
      = app->root()->clicked().connect(this, &WDatePicker::done);
  }
}

void WDatePicker::done()
{
  setPopupVisible(false);
}

void WDatePicker::setGlobalPopup(bool global)
{
  global_ = global;
}

void WDatePicker::setFormat(const WT_USTRING& format)
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(validator());
  if (dv)
    dv->setFormat(format);
}

const WT_USTRING& WDatePicker::format() const
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(validator());
  if (dv)
    return dv->format();
  else
    return WT_USTRING::Empty;
}

void WDatePicker::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    const WDate& calDate = *calendar_->selection().begin();

    WT_USTRING s = calDate.toString(format());
    if (s != text()) {
      setText(calDate.toString(format()));
      changed().emit();
    }
  }
}

WDate WDatePicker::date() const
{
  return WDate::fromString(text(), format());
}

void WDatePicker::setDate(const WDate& date)
{
  if (!date.isNull()) {
    setText(date.toString(format()));
    calendar_->select(date);
    calendar_->browseTo(date);
  }
}

void WDatePicker::setFromLineEdit()
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

int WDatePicker::boxPadding(Orientation orientation) const
{
  if (!nativeControl() && orientation == Horizontal)
    return WLineEdit::boxPadding(orientation) + 8; // Half since for one side
  else
    return WLineEdit::boxPadding(orientation);
}

void WDatePicker::setEnabled(bool enabled)
{
  setDisabled(!enabled);
}

void WDatePicker::propagateSetEnabled(bool enabled)
{
  setPopupVisible(false);

  WLineEdit::propagateSetEnabled(enabled);
}

void WDatePicker::setHidden(bool hidden, const WAnimation& animation)
{
  WLineEdit::setHidden(hidden, animation);

  if (hidden)
    setPopupVisible(false);
}

void WDatePicker::setBottom(const WDate& bottom)
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(validator());
  if (dv) {
    dv->setBottom(bottom);
    calendar_->setBottom(bottom);
  }
}

WDate WDatePicker::bottom() const
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(validator());
  if (dv)
    return dv->bottom();
  else 
    return WDate();
}
  
void WDatePicker::setTop(const WDate& top) 
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(validator());
  if (dv) {
    dv->setTop(top);
    calendar_->setTop(top);
  }
}

WDate WDatePicker::top() const
{
  WDateValidator *dv = dynamic_cast<WDateValidator *>(validator());
  if (dv)
    return dv->top();
  else 
    return WDate();
}
}
