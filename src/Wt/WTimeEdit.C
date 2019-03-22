
#include "Wt/WTimeEdit.h"

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WPopupWidget.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"
#include "Wt/WTheme.h"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WTimeEdit.min.js"
#endif

namespace Wt {

LOGGER("WTimeEdit");

WTimeEdit::WTimeEdit()
  : WLineEdit()
{
  setValidator(std::shared_ptr<WTimeValidator>(new WTimeValidator()));
  changed().connect(this, &WTimeEdit::setFromLineEdit);

  timePicker_ = new WTimePicker(this);
  timePicker_->selectionChanged().connect(this, &WTimeEdit::setFromTimePicker);
  timePicker_->setWrapAroundEnabled(true);
}

WTimeEdit::~WTimeEdit()
{
  if (!popup_) {
    // timePicker_ is not owned by popup_, because it doesn't exist
    delete timePicker_;
  }
}

void WTimeEdit::load()
{
  bool wasLoaded = loaded();

  WLineEdit::load();
  // Loading of popup_ is deferred (see issue #4897)

  if (wasLoaded)
    return;

  const char *TEMPLATE = "${timePicker}";

  std::unique_ptr<WTemplate> t(new WTemplate(WString::fromUTF8(TEMPLATE)));
  t->bindWidget("timePicker", std::unique_ptr<WTimePicker>(timePicker_));
  popup_.reset(new WPopupWidget(std::move(t)));
  if (isHidden()) {
    popup_->setHidden(true);
  }
  popup_->setAnchorWidget(this);
  popup_->setTransient(true);

  WApplication::instance()
    ->theme()->apply(this, popup_.get(), TimePickerPopup);

  escapePressed().connect(popup_.get(), &WPopupWidget::hide);
  escapePressed().connect(this, &WWidget::setFocus);
}

void WTimeEdit::setTime(const WTime& time)
{
  if (!time.isNull()) {
    setText(time.toString(format()));
    timePicker_->setTime(time);
  }
}

WTime WTimeEdit::time() const
{
  return WTime::fromString(text(), format());
}

std::shared_ptr<WTimeValidator> WTimeEdit::timeValidator() const
{
  return std::dynamic_pointer_cast<WTimeValidator>(WLineEdit::validator());
}

void WTimeEdit::setFormat(const WT_USTRING& format)
{
  auto tv = timeValidator();

  if (tv) {
    WTime t = this->time();
    tv->setFormat(format);
    timePicker_->configure();
    setTime(t);
  } else
    LOG_WARN("setFormat() ignored since validator is not WTimeValidator");
}

WT_USTRING WTimeEdit::format() const
{
  auto tv = timeValidator();

  if (tv)
    return tv->format();
  else {
    LOG_WARN("format() is bogus since validator is not WTimeValidator.");
    return WT_USTRING();
  }
}

void WTimeEdit::setBottom(const WTime &bottom)
{
    auto tv = timeValidator();
    if(tv)
        tv->setBottom(bottom);
}

WTime WTimeEdit::bottom() const
{
    auto tv = timeValidator();
    if(tv)
        return tv->bottom();
    return WTime();
}

void WTimeEdit::setTop(const WTime &top)
{
    auto tv = timeValidator();
    if(tv)
        tv->setTop(top);
}

WTime WTimeEdit::top() const
{
    auto tv = timeValidator();
    if(tv)
        return tv->top();
    return WTime();
}

void WTimeEdit::setHidden(bool hidden, const WAnimation& animation)
{
  WLineEdit::setHidden(hidden, animation);
  if (popup_) {
    popup_->setHidden(hidden, animation);
  }
}

void WTimeEdit::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full))
    defineJavaScript();

  WLineEdit::render(flags);
}

void WTimeEdit::propagateSetEnabled(bool enabled)
{
  WLineEdit::propagateSetEnabled(enabled);
}

void WTimeEdit::setFromTimePicker()
{
  setTime(timePicker_->time());
  textInput().emit();
  changed().emit();
}

void WTimeEdit::setFromLineEdit()
{
  WTime t = WTime::fromString(text(), format());
  if (t.isValid())
    timePicker_->setTime(t);
}

void WTimeEdit::defineJavaScript()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/WTimeEdit.js", "WTimeEdit", wtjs1);
  std::string jsObj = "new " WT_CLASS ".WTimeEdit("
                      + app->javaScriptClass() + "," + jsRef() + ","
		      + jsStringLiteral(popup_->id()) + ");";
  setJavaScriptMember(" WTimeEdit", jsObj);
#ifdef WT_CNOR
  EventSignalBase& b = mouseMoved();
  EventSignalBase& c = keyWentDown();
#endif
  connectJavaScript(mouseMoved(), "mouseMove");
  connectJavaScript(mouseWentUp(), "mouseUp");
  connectJavaScript(mouseWentDown(), "mouseDown");
  connectJavaScript(mouseWentOut(), "mouseOut");
}

void WTimeEdit::connectJavaScript(Wt::EventSignalBase& s,
                                  const std::string& methodName)
{
  std::string jsFunction =
    "function(dobj, event) {"
    """var o = " + jsRef() + ";"
    """if(o && o.wtDObj) o.wtDObj." + methodName + "(dobj, event);"
    "}";
  s.connect(jsFunction);
}

void WTimeEdit::setHourStep(int step)
{
  timePicker_->setHourStep(step);
}

int WTimeEdit::hourStep() const
{
  return timePicker_->hourStep();
}

void WTimeEdit::setMinuteStep(int step)
{
  timePicker_->setMinuteStep(step);
}

int WTimeEdit::minuteStep() const
{
  return timePicker_->minuteStep();
}

void WTimeEdit::setSecondStep(int step)
{
  timePicker_->setSecondStep(step);
}

int WTimeEdit::secondStep() const
{
  return timePicker_->secondStep();
}

void WTimeEdit::setMillisecondStep(int step)
{
  timePicker_->setMillisecondStep(step);
}

int WTimeEdit::millisecondStep() const
{
  return timePicker_->millisecondStep();
}

void WTimeEdit::setWrapAroundEnabled(bool enabled)
{
  timePicker_->setWrapAroundEnabled(enabled);
}

bool WTimeEdit::wrapAroundEnabled() const
{
  return timePicker_->wrapAroundEnabled();
}
  

}
