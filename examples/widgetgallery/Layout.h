// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LAYOUT_H_
#define LAYOUT_H_

#include "TopicWidget.h"

class Layout : public TopicWidget
{
public:
  Layout();

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *containers();
  Wt::WWidget *templates();
  Wt::WWidget *text();
  Wt::WWidget *grouping();
  Wt::WWidget *layoutManagers();
  Wt::WWidget *dialogs();
  Wt::WWidget *images();
  Wt::WWidget *css();
  Wt::WWidget *themes();
  Wt::WWidget *loadingIndicator();
  void loadingIndicatorSelected(Wt::WString indicator);
};

#endif // LAYOUT_H_
