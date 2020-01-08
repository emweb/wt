// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef EVENTS_DEMO_H_
#define EVENTS_DEMO_H_

#include "TopicWidget.h"

class EventsDemo : public TopicWidget
{
public:
  EventsDemo();

  void populateSubMenu(Wt::WMenu *menu);

private:
  std::unique_ptr<Wt::WWidget> wKeyEvent();
  std::unique_ptr<Wt::WWidget> wMouseEvent();
  std::unique_ptr<Wt::WWidget> wDropEvent();

  void showKeyWentUp(const Wt::WKeyEvent &e);
  void showKeyWentDown(const Wt::WKeyEvent &e);
  void showKeyPressed(const Wt::WKeyEvent &e);
  void showEnterPressed();
  void showEscapePressed();
  void describe(const Wt::WKeyEvent &e);
  void setKeyType(const std::string &type, const Wt::WKeyEvent *e = 0);
  Wt::WText *keyEventType_;
  Wt::WText *keyEventDescription_;
  std::string lastKeyType_;
  unsigned int keyEventRepeatCounter_;

  void showClicked(const Wt::WMouseEvent &e);
  void showDoubleClicked(const Wt::WMouseEvent &e);
  void showMouseWentOut(const Wt::WMouseEvent &e);
  void showMouseWentOver(const Wt::WMouseEvent &e);
  void showMouseMoved(const Wt::WMouseEvent &e);
  void showMouseWentUp(const Wt::WMouseEvent &e);
  void showMouseWentDown(const Wt::WMouseEvent &e);
  void showMouseWheel(const Wt::WMouseEvent &e);

  Wt::WText *mouseEventType_;
  Wt::WText *mouseEventDescription_;

  void describe(const Wt::WMouseEvent &e);
};

#endif

