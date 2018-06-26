/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WTimer.h"
#include "Wt/WTimerWidget.h"
#include "Wt/WContainerWidget.h"
#include "TimeUtil.h"

#include <algorithm>

namespace Wt {

WTimer::WTimer()
  : uTimerWidget_(new WTimerWidget(this)),
    singleShot_(false),
    interval_(0),
    active_(false),
    timeoutConnected_(false),
    timeout_(new Time())
{
  timerWidget_ = uTimerWidget_.get();
}

EventSignal<WMouseEvent>& WTimer::timeout()
{
  return timerWidget_->clicked();
}

WTimer::~WTimer()
{
  if (active_)
    stop();
}

void WTimer::setInterval(std::chrono::milliseconds msec)
{
  interval_ = msec;
}

void WTimer::setSingleShot(bool singleShot)
{
  singleShot_ = singleShot;
}

void WTimer::start()
{
  if (!active_) {
    WApplication *app = WApplication::instance();    
    if (app && app->timerRoot())
      app->timerRoot()->addWidget(std::move(uTimerWidget_));
  }

  active_ = true;
  *timeout_ = Time() + interval_.count();

  bool jsRepeat = !timeout().isExposedSignal() && !singleShot_;

  timerWidget_->timerStart(jsRepeat);

  if (!timeoutConnected_) {
    timeout().connect(this, &WTimer::gotTimeout);
    timeoutConnected_ = true;
  }
}

void WTimer::stop()
{
  if (active_) {
    if (timerWidget_ && timerWidget_->parent()) {
      uTimerWidget_ = std::unique_ptr<WTimerWidget>(static_cast<WTimerWidget*>(
                        timerWidget_->parent()->removeWidget(timerWidget_.get()).release()));

    }
    active_ = false;
  }
}

void WTimer::gotTimeout()
{
  if (active_) {
    if (!singleShot_) {
      *timeout_ = Time() + interval_.count();
      timerWidget_->timerStart(false);    
    } else
      stop();
  }
}

int WTimer::getRemainingInterval() const
{
  int remaining = *timeout_ - Time();
  return std::max(0, remaining);
}

}
