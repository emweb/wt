// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHARTWIDGETS_H_
#define CHARTWIDGETS_H_

#include "ControlsWidget.h"

class ChartWidgets : public ControlsWidget
{
public:
  ChartWidgets(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *category();
  Wt::WWidget *scatterplot();
  Wt::WWidget *pie();
};

#endif
