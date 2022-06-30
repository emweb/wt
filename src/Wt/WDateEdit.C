/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDateEdit.h"

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WDateValidator.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WPopupWidget.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"
#include "Wt/WTheme.h"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WDateEdit.min.js"
#endif

namespace {
  const std::string YMD_FORMAT = "yyyy-MM-dd";
}

namespace Wt {

LOGGER("WDateEdit");

WDateEdit::WDateEdit()
  : customFormat_(false),
    nativeControl_(false)
{
  changed().connect(this, &WDateEdit::setFromLineEdit);

  init();
}

WDateEdit::~WDateEdit()
{ }

void WDateEdit::load()
{
  bool wasLoaded = loaded();

  WLineEdit::load();
  // Loading of popup_ is deferred (see issue #4897)

  if (wasLoaded || nativeControl())
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

  oCalendar_->activated().connect(popup_.get(), &WPopupWidget::hide);
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

void WDateEdit::setNativeControl(bool nativeControl)
{
  // Specific set format normalized by the browser:
  // https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/date
  setFormat(YMD_FORMAT);
  nativeControl_ = nativeControl;

  if (nativeControl) {
    uCalendar_.reset(nullptr);
    popup_.reset(nullptr);
  } else {
    // Reset loaded state, so a  new popup and signals can be created.
    WWebWidget::flags_.reset(BIT_LOADED);

    init();
    load();
  }
}

void WDateEdit::setFormat(const WT_USTRING& format)
{
  std::shared_ptr<WDateValidator> dv = dateValidator();

  if (dv) {
    if (!nativeControl()) {
      WDate d = this->date();
      dv->setFormat(format);
      setDate(d);
      customFormat_ = true;
    } else {
      LOG_WARN("setFormat() ignored since nativeControl() is true");
    }
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
  if (nativeControl()) {
    return;
  }

  if (!oCalendar_->selection().empty()) {
    WDate calDate = Utils::first(oCalendar_->selection());
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
  if (nativeControl()) {
    setText(date.toString(format()));
    return;
  }

  if (!date.isNull()) {
    setText(date.toString(format()));
    oCalendar_->select(date);
    oCalendar_->browseTo(date);
  }
}

void WDateEdit::setFromLineEdit()
{
  if (nativeControl()) {
    return;
  }

  WDate d = WDate::fromString(text(), format());

  if (d.isValid()) {
    if (oCalendar_->selection().empty()) {
      oCalendar_->select(d);
      oCalendar_->selectionChanged().emit();
    } else {
      WDate j = Utils::first(oCalendar_->selection());

      if (j != d) {
        oCalendar_->select(d);
        oCalendar_->selectionChanged().emit();
      }
    }

    oCalendar_->browseTo(d);
  }
}

void WDateEdit::propagateSetEnabled(bool enabled)
{
  WLineEdit::propagateSetEnabled(enabled);
}

void WDateEdit::validatorChanged()
{
  auto dv = dateValidator();
  if (dv && !nativeControl()) {
    oCalendar_->setBottom(dv->bottom());
    oCalendar_->setTop(dv->top());
  }
  WLineEdit::validatorChanged();
}

std::string WDateEdit::type() const noexcept
{
  return nativeControl() ? "date" : WLineEdit::type();
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
  if (dv) {
    dv->setBottom(bottom);
    // validatorChanged will take care of the calendar
  } else if (!nativeControl()) {
    oCalendar_->setBottom(bottom);
  }
}

WDate WDateEdit::bottom() const
{
  if (nativeControl()) {
    std::shared_ptr<WDateValidator> dv = dateValidator();
    if (dv) {
      return dv->bottom();
    }

    return WDate();
  }

  return oCalendar_->bottom();
}

void WDateEdit::setTop(const WDate& top)
{
  std::shared_ptr<WDateValidator> dv = dateValidator();
  if (dv) {
    dv->setTop(top);
    // validatorChanged will take care of the calendar
  } else if (!nativeControl()) {
    oCalendar_->setTop(top);
  }
}

WDate WDateEdit::top() const
{
  if (nativeControl()) {
    std::shared_ptr<WDateValidator> dv = dateValidator();
    if (dv) {
      return dv->top();
    }

    return WDate();
  }

  return oCalendar_->top();
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

void WDateEdit::init()
{
  uCalendar_ = std::make_unique<WCalendar>();
  oCalendar_ = uCalendar_.get();
  oCalendar_->setSingleClickSelect(true);
  oCalendar_->activated().connect(this, &WDateEdit::setFocusTrue);
  oCalendar_->selectionChanged().connect(this, &WDateEdit::setFromCalendar);

  setValidator(std::make_shared<WDateValidator>(WApplication::instance()->locale().dateFormat()));
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
  if (flags.test(RenderFlag::Full) && !nativeControl()) {
    defineJavaScript();
    std::shared_ptr<WDateValidator> dv = dateValidator();
    if (dv) {
      setTop(dv->top());
      setBottom(dv->bottom());
    }
  }

  WLineEdit::render(flags);
}

void WDateEdit::updateDom(DomElement& element, const bool all)
{
  if (nativeControl() && hasValidatorChanged()) {
    auto dv = dateValidator();
    if (dv) {
      auto bottom = dv->bottom();
      if (bottom.isValid()) {
        element.setAttribute("min", bottom.toString(YMD_FORMAT).toUTF8());
      } else {
        element.removeAttribute("min");
      }
      auto top = dv->top();
      if (top.isValid()) {
        element.setAttribute("max", top.toString(YMD_FORMAT).toUTF8());
      } else {
        element.removeAttribute("max");
      }
    }
  }
  WLineEdit::updateDom(element, all);
}

void WDateEdit::setFocusTrue()
{
  setFocus(true);
}

}
