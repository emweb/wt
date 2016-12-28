// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba
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
  std::unique_ptr<WWidget> internalPaths();
  std::unique_ptr<WWidget> anchor();
  std::unique_ptr<WWidget> stackedWidget();
  std::unique_ptr<WWidget> tabWidget();
  std::unique_ptr<WWidget> menuWidget();
  std::unique_ptr<WWidget> navigationBar();
  std::unique_ptr<WWidget> popupMenu();
  std::unique_ptr<WWidget> splitButton();
  std::unique_ptr<WWidget> toolBar();
};

#endif // NAVIGATION_H_
