/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WDateTimeEdit.h"
#include "Wt/WDateTimeValidator.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WTemplate.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WDateTimeEdit.min.js"
#endif

namespace {
  const char* YMD_HMS_FORMAT = "yyyy-MM-dd'T'HH:mm";

  inline WDate toDate(const WDateTime& dt) {
#ifndef WT_TARGET_JAVA
    return dt.date();
#else
    return dt.date().isNull() ? WDate() : WDate(dt.date().year(), dt.date().month(), dt.date().day());
#endif
  }
}

namespace Wt {

LOGGER("WDateTimeEdit");

WDateTimeEdit::WDateTimeEdit()
  : customFormat_(false),
    nativeControl_(false)
{
  changed().connect(this, &WDateTimeEdit::setFromLineEdit);

  uCalendar_ = std::make_unique<WCalendar>();
  calendar_ = uCalendar_.get();
  calendar_->setSingleClickSelect(true);
  calendar_->activated().connect(this, &WDateTimeEdit::setFocusTrue);
  calendar_->selectionChanged().connect(this, &WDateTimeEdit::setFromCalendar);

  setValidator(std::shared_ptr<WDateTimeValidator>(new WDateTimeValidator()));

  uTimePicker_ = std::make_unique<WTimePicker>(this);
  timePicker_ = uTimePicker_.get();
  timePicker_->selectionChanged().connect(this, &WDateTimeEdit::setFromTimePicker);
  timePicker_->setWrapAroundEnabled(true);

  if (WWebWidget::defaultNativeControl()) {
    setNativeControl(true);
  }
};

WDateTimeEdit::~WDateTimeEdit()
{
  calendar_ = nullptr;
  timePicker_ = nullptr;
}

void WDateTimeEdit::setDateTime(const WDateTime& dateTime)
{
  if (!dateTime.isNull()) {
    setText(dateTime.toString(format()));

    if (dateTime.isValid()) {
      WDate date = toDate(dateTime);
      calendar_->select(date);
      calendar_->browseTo(date);
      timePicker_->setTime(dateTime.time());
    }
  }
}

WDateTime WDateTimeEdit::dateTime() const
{
  return WDateTime::fromString(text(), format());
}

std::shared_ptr<WDateTimeValidator> WDateTimeEdit::dateTimeValidator() const
{
  return std::dynamic_pointer_cast<WDateTimeValidator>(WLineEdit::validator());
}

void WDateTimeEdit::setNativeControl(bool nativeControl)
{
  setFormat(YMD_HMS_FORMAT);
  nativeControl_ = nativeControl;
}

void WDateTimeEdit::setFormat(const WT_USTRING& format)
{
  std::shared_ptr<WDateTimeValidator> dtv = dateTimeValidator();

  std::string formatStr = format.toUTF8();
  std::size_t timeZoneIndex = formatStr.find("Z");
  if (timeZoneIndex != std::string::npos) {
    formatStr.erase(timeZoneIndex, 1);
    LOG_WARN("setFormat(): contains a timezone, which is omitted");
  }

  if (dtv) {
    if (!nativeControl()) {
      WDateTime dt = this->dateTime();
      dtv->setFormat(formatStr);
      timePicker_->configure();
      setDateTime(dt);
      customFormat_ = true;
    } else {
      LOG_WARN("setFormat() ignored since nativeControl() is true");
    }
  } else {
    LOG_WARN("setFormat(): ignored since validator is not a WDateTimeValidator");
  }
}

WT_USTRING WDateTimeEdit::format() const
{
  std::shared_ptr<WDateTimeValidator> dtv = dateTimeValidator();

  if (dtv) {
    return dtv->format();
  } else {
    LOG_WARN("format(): is bogus since validator is not a WDateTimeValidator");
    return WT_USTRING();
  }
}

void WDateTimeEdit::setBottom(const WDateTime& bottom)
{
  std::shared_ptr<WDateTimeValidator> dtv = dateTimeValidator();
  if (dtv) {
    dtv->setBottom(bottom);
  }

  calendar_->setBottom(toDate(bottom));
}

WDateTime WDateTimeEdit::bottom() const
{
  auto dtv = dateTimeValidator();
  if (dtv) {
      return dtv->bottom();
  }

  return WDateTime();
}

void WDateTimeEdit::setTop(const WDateTime& top)
{
  std::shared_ptr<WDateTimeValidator> dtv = dateTimeValidator();
  if (dtv) {
    dtv->setTop(top);
  }

  calendar_->setTop(top.date());
}

WDateTime WDateTimeEdit::top() const
{
  auto dtv = dateTimeValidator();
  if (dtv) {
      return dtv->top();
  }

  return WDateTime();
}

void WDateTimeEdit::browseToPreviousYear()
{
  calendar_->browseToPreviousYear();
}

void WDateTimeEdit::browseToPreviousMonth()
{
  calendar_->browseToPreviousMonth();
}

void WDateTimeEdit::browseToNextYear()
{
  calendar_->browseToNextYear();
}

void WDateTimeEdit::browseToNextMonth()
{
  calendar_->browseToNextMonth();
}

void WDateTimeEdit::browseTo(const WDate& date)
{
  calendar_->browseTo(date);
}

int WDateTimeEdit::currentYear() const
{
  return calendar_->currentYear();
}

int WDateTimeEdit::currentMonth() const
{
  return calendar_->currentMonth();
}

void WDateTimeEdit::setHorizontalHeaderFormat(CalendarHeaderFormat format)
{
  calendar_->setHorizontalHeaderFormat(format);
}

CalendarHeaderFormat WDateTimeEdit::horizontalHeaderFormat()
{
  return calendar_->horizontalHeaderFormat();
}

void WDateTimeEdit::setFirstDayOfWeek(int dayOfWeek)
{
  calendar_->setFirstDayOfWeek(dayOfWeek);
}

void WDateTimeEdit::setHourStep(int step)
{
  timePicker_->setHourStep(step);
}

int WDateTimeEdit::hourStep() const
{
  return timePicker_->hourStep();
}

void WDateTimeEdit::setMinuteStep(int step)
{
  timePicker_->setMinuteStep(step);
}

int WDateTimeEdit::minuteStep() const
{
  return timePicker_->minuteStep();
}

void WDateTimeEdit::setSecondStep(int step)
{
  timePicker_->setSecondStep(step);
}

int WDateTimeEdit::secondStep() const
{
  return timePicker_->secondStep();
}

void WDateTimeEdit::setMillisecondStep(int step)
{
  timePicker_->setMillisecondStep(step);
}

int WDateTimeEdit::millisecondStep() const
{
  return timePicker_->millisecondStep();
}

void WDateTimeEdit::setWrapAroundEnabled(bool rollover)
{
  timePicker_->setWrapAroundEnabled(rollover);
}

bool WDateTimeEdit::wrapAroundEnabled() const
{
  return timePicker_->wrapAroundEnabled();
}

void WDateTimeEdit::setHidden(bool hidden,
                              const WAnimation& animation)
{
  WLineEdit::setHidden(hidden, animation);
  // rationale: when calling hide(), line edit and popup should go away. When
  // calling show(), line edit becomes visible, but popup is only shown when
  // using the widget.
  if (uPopup_ && hidden) {
    uPopup_->setHidden(hidden, animation);
  }
}

void WDateTimeEdit::load()
{
  bool wasLoaded = loaded();

  WLineEdit::load();
  // Loading of popup_ is deferred (see issue #4897)

  if (wasLoaded) {
    return;
  }

  std::unique_ptr<WTemplate> t(new WTemplate(WString::tr("Wt.WDateTimeEdit.popup")));
  WTemplate *templ = t.get();

  templ->addFunction("tr", &WTemplate::Functions::tr);
  templ->bindWidget("calendar", std::move(uCalendar_));
  templ->bindWidget("timePicker", std::move(uTimePicker_));


  uPopup_.reset(new WPopupWidget(std::move(t)));
  if (isHidden()) {
    uPopup_->setHidden(true);
  }
  uPopup_->setAnchorWidget(this);
  uPopup_->setTransient(true);

  scheduleThemeStyleApply(WApplication::instance()->theme(), uPopup_.get(), DateTimePickerPopup);

  escapePressed().connect(uPopup_.get(), &WPopupWidget::hide);
  escapePressed().connect(this, &WDateTimeEdit::setFocusTrue);
}

void WDateTimeEdit::refresh()
{
  WLineEdit::refresh();

  auto dtv = dateTimeValidator();

  if (!customFormat_ && dtv) {
    WDateTime dt = this->dateTime();
    dtv->setFormat(WLocale::currentLocale().dateTimeFormat());
    setDateTime(dt);
    timePicker_->configure();
  }
}

void WDateTimeEdit::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full) && !nativeControl()) {
    defineJavaScript();
    std::shared_ptr<WDateTimeValidator> dtv = dateTimeValidator();
    if (dtv) {
      setTop(dtv->top());
      setBottom(dtv->bottom());
    }
  }
  WLineEdit::render(flags);
}

void WDateTimeEdit::updateDom(DomElement& element, bool all)
{
  if (nativeControl() && hasValidatorChanged()) {
    auto dtv = dateTimeValidator();
    if (dtv) {
      auto bottom = dtv->bottom();
      if (bottom.isValid()) {
        element.setAttribute("min", bottom.toString(YMD_HMS_FORMAT).toUTF8());
      } else {
        element.removeAttribute("min");
      }
      auto top = dtv->top();
      if (top.isValid()) {
        element.setAttribute("max", top.toString(YMD_HMS_FORMAT).toUTF8());
      } else {
        element.removeAttribute("max");
      }
    }
  }
  WLineEdit::updateDom(element, all);
}

void WDateTimeEdit::validatorChanged()
{
  auto dtv = dateTimeValidator();
  if (dtv) {
    calendar_->setBottom(toDate(dtv->bottom()));
    calendar_->setTop(dtv->top().date());
    if (loaded()) {
      timePicker_->configure();
    }
  }  else {
    LOG_WARN("Validator changed to a non-WDateTimeValidator. Several functions may not work correctly.");
  }

  WLineEdit::validatorChanged();
}

std::string WDateTimeEdit::type() const noexcept
{
  return nativeControl() ? "datetime-local" : WLineEdit::type();
}

void WDateTimeEdit::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WDateTimeEdit.js", "WDateTimeEdit", wtjs1);

  std::string jsObj = "new " WT_CLASS ".WDateTimeEdit("
    + app->javaScriptClass() + "," + jsRef() + ","
    + jsStringLiteral(uPopup_->id()) + ");";

  setJavaScriptMember(" WDateTimeEdit", jsObj);

#ifdef WT_CNOR
  EventSignalBase& b = mouseMoved();
  EventSignalBase& c = keyWentDown();
#endif

  connectJavaScript(mouseMoved(), "mouseMove");
  connectJavaScript(mouseWentUp(), "mouseUp");
  connectJavaScript(mouseWentDown(), "mouseDown");
  connectJavaScript(mouseWentOut(), "mouseOut");
}

void WDateTimeEdit::connectJavaScript(Wt::EventSignalBase& s,
          const std::string& methodName)
{
  std::string jsFunction =
    "function(dobj, event) {"
    """var o = " + jsRef() + ";"
    """if (o && o.wtDObj) o.wtDObj." + methodName + "(dobj, event);"
    "}";

  s.connect(jsFunction);
}

void WDateTimeEdit::setFocusTrue()
{
  setFocus(true);
}

void WDateTimeEdit::setFromCalendar()
{
  if (!calendar_->selection().empty()) {
    setDateTime(WDateTime(Utils::first(calendar_->selection()),
                          timePicker_->time()));
    textInput().emit();
    changed().emit();
  }
}

void WDateTimeEdit::setFromTimePicker()
{
  bool missingDate = calendar_->selection().empty();
  WDate date = missingDate ? WDate() : Utils::first(calendar_->selection());
  setDateTime(WDateTime(date, timePicker_->time()));

  if (!missingDate) {
    textInput().emit();
  }

  changed().emit();
}

void WDateTimeEdit::setFromLineEdit()
{
  if (nativeControl()) {
    return;
  }

  WDateTime dt = WDateTime::fromString(text(), format());
  WTime t = dt.time();
  WDate d = toDate(dt);
  if (t.isValid()) {
    timePicker_->setTime(t);
  }

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

}
