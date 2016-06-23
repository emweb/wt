
#include "Wt/WTimeEdit"

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WLineEdit"
#include "Wt/WLogger"
#include "Wt/WPopupWidget"
#include "Wt/WPushButton"
#include "Wt/WTemplate"
#include "Wt/WTheme"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WTimeEdit.min.js"
#endif

namespace Wt {

LOGGER("WTimeEdit");

WTimeEdit::WTimeEdit(WContainerWidget *parent)
  : WLineEdit(parent),
    popup_(0)
{
  setValidator(new WTimeValidator(this));
  changed().connect(this, &WTimeEdit::setFromLineEdit);

  timePicker_ = new WTimePicker(this);
  timePicker_->selectionChanged().connect(this, &WTimeEdit::setFromTimePicker);
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
  WTemplate *t = new WTemplate(WString::fromUTF8(TEMPLATE));
  popup_ = new WPopupWidget(t, this);
  if (isHidden()) {
    popup_->setHidden(true);
  }
  popup_->setAnchorWidget(this);
  popup_->setTransient(true);

  t->bindWidget("timePicker", timePicker_);

  WApplication::instance()->theme()->apply(this, popup_, TimePickerPopupRole);

  escapePressed().connect(popup_, &WPopupWidget::hide);
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

WTimeValidator *WTimeEdit::validator() const
{
  return dynamic_cast<WTimeValidator *>(WLineEdit::validator());
}

void WTimeEdit::setFormat(const WT_USTRING& format)
{
  WTimeValidator *tv = validator();

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
  WTimeValidator *tv = validator();

  if (tv)
    return tv->format();
  else {
    LOG_WARN("format() is bogus since validator is not WTimeValidator.");
    return WT_USTRING();
  }
}

void WTimeEdit::setBottom(const WTime &bottom)
{
    WTimeValidator *tv = validator();
    if(tv)
        tv->setBottom(bottom);
}

WTime WTimeEdit::bottom() const
{
    WTimeValidator *tv = validator();
    if(tv)
        return tv->bottom();
    return WTime();
}

void WTimeEdit::setTop(const WTime &top)
{
    WTimeValidator *tv = validator();
    if(tv)
        tv->setTop(top);
}

WTime WTimeEdit::top() const
{
    WTimeValidator *tv = validator();
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
  if (flags & RenderFull)
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
    """var o = jQuery.data(" + jsRef() + ", 'dobj');"
    """if(o) o." + methodName + "(dobj, event);"
    "}";
  s.connect(jsFunction);
}

}
