// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium
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
  std::unique_ptr<Wt::WWidget> containers();
  std::unique_ptr<Wt::WWidget> templates();
  std::unique_ptr<Wt::WWidget> text();
  std::unique_ptr<Wt::WWidget> grouping();
  std::unique_ptr<Wt::WWidget> layoutManagers();
  std::unique_ptr<Wt::WWidget> dialogs();
  std::unique_ptr<Wt::WWidget> images();
  std::unique_ptr<Wt::WWidget> css();
  std::unique_ptr<Wt::WWidget> themes();
  Wt::WWidget *loadingIndicator();
  void loadingIndicatorSelected(Wt::WString indicator);
};

#endif // LAYOUT_H_
