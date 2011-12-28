/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WTimerWidget"
#include "Wt/WTimer"
#include "DomElement.h"

namespace Wt {

WTimerWidget::WTimerWidget(WTimer *timer)
  : timer_(timer),
    timerStarted_(false)
{ }

WTimerWidget::~WTimerWidget()
{
  timer_->timerWidget_ = 0;
}

std::string WTimerWidget::renderRemoveJs()
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
    element.setTimeout(timer_->getRemainingInterval(), jsRepeat_);

    timerStarted_ = false;
  }

  WInteractWidget::updateDom(element, all);
}

DomElementType WTimerWidget::domElementType() const
{
  return DomElement_SPAN;
}

}
