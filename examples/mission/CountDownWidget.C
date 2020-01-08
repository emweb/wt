/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WTimer.h>
#include "CountDownWidget.h"
#include <string>
#include <algorithm>

CountDownWidget::CountDownWidget(int start, int stop, std::chrono::milliseconds msec)
  : WText(),
    done_(),
    start_(start),
    stop_(stop)
{
  stop_ = std::min(start_ - 1, stop_);  // stop must be smaller than start
  current_ = start_;

  timer_ = cpp14::make_unique<WTimer>();
  timer_->setInterval(msec);
  timer_->timeout().connect(this, &CountDownWidget::timerTick);
  timer_->start();

  setText(std::to_string(current_));
}

void CountDownWidget::cancel()
{
  timer_->stop();
}

void CountDownWidget::timerTick()
{
  setText(std::to_string(--current_));

  if (current_ <= stop_) {
    timer_->stop();
    done_.emit();
  }
}
