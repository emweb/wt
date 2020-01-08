// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTIMERWIDGET_
#define WTIMERWIDGET_

#include <Wt/WInteractWidget.h>

namespace Wt {

class WTimer;

/*
 * Wt-private widget that is created in conjunction with
 * WTimer widget. It's clicked signal serves as the event signal
 * that will be activated when the event expires.
 */
class WT_API WTimerWidget : public WInteractWidget
{
public:
  WTimerWidget(WTimer *timer);
  ~WTimerWidget();

  void timerStart(bool jsRepeat);
  bool timerExpired();

  bool jsRepeat() const { return jsRepeat_; }

private:
  WTimer *timer_;
  bool timerStarted_;
  bool jsRepeat_;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual std::string renderRemoveJs(bool recursive) override;

  virtual void enableAjax() override;

  friend class WebSession;
};

}

#endif // WTIMERWIDGET_
