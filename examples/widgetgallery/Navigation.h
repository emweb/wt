// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NAVIGATION_H_
#define NAVIGATION_H_

#include "TopicWidget.h"

class Navigation : public TopicWidget
{
public:
  Navigation();

  void populateSubMenu(Wt::WMenu *menu);

private:
  std::unique_ptr<Wt::WWidget> internalPaths();
  std::unique_ptr<Wt::WWidget> anchor();
  std::unique_ptr<Wt::WWidget> stackedWidget();
  std::unique_ptr<Wt::WWidget> tabWidget();
  std::unique_ptr<Wt::WWidget> menuWidget();
  std::unique_ptr<Wt::WWidget> navigationBar();
  std::unique_ptr<Wt::WWidget> popupMenu();
  std::unique_ptr<Wt::WWidget> splitButton();
  std::unique_ptr<Wt::WWidget> toolBar();
};

#endif // NAVIGATION_H_
