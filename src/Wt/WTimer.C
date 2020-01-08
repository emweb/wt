/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
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
  WApplication *app = WApplication::instance();
  if (!active_) {
    if (app && app->timerRoot())
      app->timerRoot()->addWidget(std::move(uTimerWidget_));
  }

  active_ = true;
  *timeout_ = Time() + static_cast<int>(interval_.count());

  bool jsRepeat = !singleShot_ &&
                  ((app && app->environment().ajax()) ||
                   !timeout().isExposedSignal());

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
      uTimerWidget_ = timerWidget_->parent()->removeWidget(timerWidget_.get());
    }
    active_ = false;
  }
}

void WTimer::gotTimeout()
{
  if (active_) {
    if (!singleShot_) {
      *timeout_ = Time() + static_cast<int>(interval_.count());
      if (!timerWidget_->jsRepeat()) {
        WApplication *app = WApplication::instance();
        timerWidget_->timerStart(app->environment().ajax());
      }
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
