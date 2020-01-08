/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WTimerWidget.h"
#include "Wt/WTimer.h"
#include "DomElement.h"

namespace Wt {

WTimerWidget::WTimerWidget(WTimer *timer)
  : timer_(timer),
    timerStarted_(false)
{ }

WTimerWidget::~WTimerWidget()
{
  timer_->timerWidget_ = nullptr;
}

std::string WTimerWidget::renderRemoveJs(bool recursive)
{
 return "{"
   "var obj=" + jsRef() + ";"
   "if (obj && obj.timer) {"
   """clearTimeout(obj.timer);"
   """obj.timer = null;"
   "}" WT_CLASS ".remove('" + id() + "');}";
}

void WTimerWidget::timerStart(bool jsRepeat)
{
  timerStarted_ = true;
  jsRepeat_ = jsRepeat;

  repaint();
}

void WTimerWidget::enableAjax()
{
  if (timer_->isActive()) {
    timerStarted_ = true;
    repaint();
  }

  WInteractWidget::enableAjax();
}

bool WTimerWidget::timerExpired()
{
  return timer_->getRemainingInterval() == 0;
}

void WTimerWidget::updateDom(DomElement& element, bool all)
{
  if (timerStarted_
      || ((!WApplication::instance()->environment().javaScript() || all)
	  && timer_->isActive())) {
    if (jsRepeat_)
      element.setTimeout(timer_->getRemainingInterval(), static_cast<int>(timer_->interval().count()));
    else
      element.setTimeout(timer_->getRemainingInterval(), false);

    timerStarted_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

DomElementType WTimerWidget::domElementType() const
{
  return DomElementType::SPAN;
}

}
