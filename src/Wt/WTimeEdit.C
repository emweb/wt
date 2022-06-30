
#include "Wt/WTimeEdit.h"

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WPopupWidget.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"
#include "Wt/WTheme.h"

#include "DomElement.h"
#include "WebUtils.h"
#include <chrono>

#ifndef WT_DEBUG_JS
#include "js/WTimeEdit.min.js"
#endif

namespace {
  const std::string HM_FORMAT = "HH:mm";
  const std::string HMS_FORMAT = "HH:mm:ss";
}

namespace Wt {

LOGGER("WTimeEdit");

WTimeEdit::WTimeEdit()
  : WLineEdit(),
    nativeControl_(false)
{
  changed().connect(this, &WTimeEdit::setFromLineEdit);

  init();
}

WTimeEdit::~WTimeEdit()
{}

void WTimeEdit::load()
{
  bool wasLoaded = loaded();

  WLineEdit::load();
  // Loading of popup_ is deferred (see issue #4897)

  if (wasLoaded)
    return;

  const char *TEMPLATE = "${timePicker}";

  std::unique_ptr<WTemplate> t(new WTemplate(WString::fromUTF8(TEMPLATE)));
  t->bindWidget("timePicker", std::move(uTimePicker_));
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

void WTimeEdit::setNativeControl(bool nativeControl)
{
  auto tv = timeValidator();

  // Specific set format normalized by the browser:
  // https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/time
  if (nativeControl && (format() == HM_FORMAT || format() == HMS_FORMAT)) {
    if (tv) {
      tv->setFormat(format());
      if (format() == HM_FORMAT) {
        tv->setStep(std::chrono::seconds(60));
      } else if (format() == HMS_FORMAT) {
        tv->setStep(std::chrono::seconds(1));
      }
    }
  } else if (nativeControl) {
    setFormat(HM_FORMAT);
  }

  if (nativeControl) {
    uTimePicker_.reset(nullptr);
  } else {
    init();
  }

  nativeControl_ = nativeControl;
}

void WTimeEdit::setTime(const WTime& time)
{
  if (!time.isNull()) {
    setText(time.toString(format()));

    if (!nativeControl()) {
      oTimePicker_->setTime(time);
    }
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
    if (nativeControl() &&
        !(format == HM_FORMAT ||
          format == HMS_FORMAT)) {
      LOG_WARN("setFormat() ignored since nativeControl() is true and the format isn't "
               "HH:mm, or HH:mm:ss");
      return;
    }

    WTime t = this->time();
    tv->setFormat(format);

    if (!nativeControl()) {
      oTimePicker_->configure();
    }

    setTime(t);
  } else {
    LOG_WARN("setFormat() ignored since validator is not WTimeValidator");
  }
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
  if (flags.test(RenderFlag::Full) && !nativeControl())
      defineJavaScript();
  WLineEdit::render(flags);
}

void WTimeEdit::updateDom(DomElement& element, bool all)
{
  const auto step = this->step();
  if (step) {
    element.setAttribute("step", step);
  }

  if (nativeControl() && hasValidatorChanged()) {
    auto tv = timeValidator();
    const auto bottom = tv->bottom();
    if (bottom.isValid()) {
      element.setAttribute("min", bottom.toString(format()).toUTF8());
    } else {
      element.removeAttribute("min");
    }
    const auto top = tv->top();
    if (top.isValid()) {
      element.setAttribute("max", top.toString(format()).toUTF8());
    } else {
      element.removeAttribute("max");
    }
  }

  WLineEdit::updateDom(element, all);
}

void WTimeEdit::validatorChanged()
{
  auto tv = timeValidator();
  Wt::WTime currentValue;

  if (tv) {
    currentValue = Wt::WTime::fromString(text());
    if (!currentValue.isValid()) {
      currentValue = Wt::WTime::fromString(text(), HM_FORMAT);
    }

    if (oTimePicker_) {
      setFormat(tv->format());
    }

    if (step()) {
      if (std::string(step()) == "60") {
        tv->setStep(std::chrono::seconds(60));
      } else if (std::string(step()) == "1") {
        tv->setStep(std::chrono::seconds(1));
      }
    }
  }

  if (nativeControl()) {
    setTime(currentValue);
  }

  WLineEdit::validatorChanged();
}

std::string WTimeEdit::type() const noexcept {
  return nativeControl() ? "time" : WLineEdit::type();
}

void WTimeEdit::propagateSetEnabled(bool enabled)
{
  WLineEdit::propagateSetEnabled(enabled);
}

void WTimeEdit::init()
{
  setValidator(std::shared_ptr<WTimeValidator>(new WTimeValidator()));
  uTimePicker_ = std::make_unique<WTimePicker>(this);
  oTimePicker_ = uTimePicker_.get();
  oTimePicker_->selectionChanged().connect(this, &WTimeEdit::setFromTimePicker);
  oTimePicker_->setWrapAroundEnabled(true);
}

void WTimeEdit::setFromTimePicker()
{
  setTime(oTimePicker_->time());
  textInput().emit();
  changed().emit();
}

void WTimeEdit::setFromLineEdit()
{
  WTime t = WTime::fromString(text(), format());
  if (t.isValid() && !nativeControl()) {
    oTimePicker_->setTime(t);
  }
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

const char* WTimeEdit::step() const noexcept
{
  if (!nativeControl()) {
    return nullptr;
  }

  if (format() == HM_FORMAT) {
    return "60";
  } else if (format() == HMS_FORMAT) {
    return "1";
  } else {
    return nullptr;
  }
}

void WTimeEdit::setHourStep(int step)
{
  if (nativeControl()) {
    return;
  }

  oTimePicker_->setHourStep(step);
}

int WTimeEdit::hourStep() const
{
  if (nativeControl()) {
    return 0;
  }

  return oTimePicker_->hourStep();
}

void WTimeEdit::setMinuteStep(int step)
{
  if (nativeControl()) {
    return;
  }

  oTimePicker_->setMinuteStep(step);
}

int WTimeEdit::minuteStep() const
{
  if (nativeControl()) {
    return 0;
  }

  return oTimePicker_->minuteStep();
}

void WTimeEdit::setSecondStep(int step)
{
  if (nativeControl()) {
    return;
  }

  oTimePicker_->setSecondStep(step);
}

int WTimeEdit::secondStep() const
{
  if (nativeControl()) {
    return 0;
  }

  return oTimePicker_->secondStep();
}

void WTimeEdit::setMillisecondStep(int step)
{
  if (nativeControl()) {
    return;
  }

  oTimePicker_->setMillisecondStep(step);
}

int WTimeEdit::millisecondStep() const
{
  if (nativeControl()) {
    return 0;
  }

  return oTimePicker_->millisecondStep();
}

void WTimeEdit::setWrapAroundEnabled(bool enabled)
{
  if (nativeControl()) {
    return;
  }

  oTimePicker_->setWrapAroundEnabled(enabled);
}

bool WTimeEdit::wrapAroundEnabled() const
{
  if (nativeControl()) {
    return true;
  }

  return oTimePicker_->wrapAroundEnabled();
}

}
