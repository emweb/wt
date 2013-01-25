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
  Wt::WWidget *internalPaths();
  Wt::WWidget *anchor();
  Wt::WWidget *stackedWidget();
  Wt::WWidget *tabWidget();
  Wt::WWidget *menuWidget();
  Wt::WWidget *navigationBar();
  Wt::WWidget *popupMenu();
  Wt::WWidget *splitButton();
  Wt::WWidget *toolBar();
};

#endif // NAVIGATION_H_
