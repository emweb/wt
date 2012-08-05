// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef BASIC_CONTROLS_H_
#define BASIC_CONTROLS_H_

#include "ControlsWidget.h"

class BasicControls : public ControlsWidget
{
public:
  BasicControls(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *wText();
  Wt::WWidget *wTemplate();
  Wt::WWidget *wBreak();
  Wt::WWidget *wAnchor();
  Wt::WWidget *wImage();
  Wt::WWidget *wTable();
  Wt::WWidget *wTree();
  Wt::WWidget *wTreeTable();
  Wt::WWidget *wPanel();
  Wt::WWidget *wTabWidget();
  Wt::WWidget *wContainerWidget();
  Wt::WWidget *wMenu();
  Wt::WWidget *wGroupBox();
  Wt::WWidget *wStackedWidget();
  Wt::WWidget *wProgressBar();
};

#endif // BASIC_CONTROLS_H_
