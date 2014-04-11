/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
// This may look like C code, but it's really -*- C++ -*-
#ifndef WCOUNTDOWNWIDGET_H_
#define WCOUNTDOWNWIDGET_H_

#include <Wt/WText>

namespace Wt {
  class WTimer;
}

using namespace Wt;

/**
 * \defgroup missionexample Timer example
 */
/*@{*/

/*!\brief A widget which displays a decrementing number.
 */
class CountDownWidget : public WText
{
public:
  /*! \brief Create a new CountDownWidget.
   *
   * The widget will count down from start to stop, decrementing
   * the number every msec milliseconds.
   */
  CountDownWidget(int start, int stop, unsigned msec,
		  WContainerWidget *parent = 0);

  /*! \brief Signal emitted when the countdown reached stop.
   */
  Wt::Signal<void>& done() { return done_; }

  /*! \brief Cancel the count down.
   */
  void cancel();

private:
  Wt::Signal<void> done_;
  int start_;
  int stop_;

  int current_;

  WTimer *timer_;

  /*! \brief Process one timer tick.
   */
  void timerTick();
};

/*@}*/

#endif // WCOUNTDOWNWIDGET_H_
