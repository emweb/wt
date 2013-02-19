/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WTimer"
#include "Wt/WTimerWidget"
#include "Wt/WContainerWidget"
#include "TimeUtil.h"

namespace Wt {

WTimer::WTimer(WObject *parent)
  : WObject(parent),
    timerWidget_(new WTimerWidget(this)),
    singleShot_(false),
    selfDestruct_(false),
    interval_(0),
    active_(false),
    timeoutConnected_(false),
    timeout_(new Time())
{ }

EventSignal<WMouseEvent>& WTimer::timeout()
{
  return timerWidget_->clicked();
}

WTimer::~WTimer()
{
  if (active_)
    stop();

  delete timerWidget_;
  delete timeout_;
}

void WTimer::setInterval(int msec)
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
      app->timerRoot()->addWidget(timerWidget_);
  }

  active_ = true;
  *timeout_ = Time() + interval_;

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
    WApplication *app = WApplication::instance();
    if (app && app->timerRoot())
      app->timerRoot()->removeWidget(timerWidget_);
    active_ = false;
  }
}

void WTimer::setSelfDestruct()
{
  selfDestruct_ = true;
}

void WTimer::gotTimeout()
{
  if (active_) {
    if (!singleShot_) {
      *timeout_ = Time() + interval_;
      timerWidget_->timerStart(false);    
    } else
      stop();
  }

  if (selfDestruct_)
    delete this;
}

int WTimer::getRemainingInterval() const
{
  int remaining = *timeout_ - Time();
  return std::max(0, remaining);
}

}
