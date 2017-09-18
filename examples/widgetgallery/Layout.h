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
  std::unique_ptr<WWidget> containers();
  std::unique_ptr<WWidget> templates();
  std::unique_ptr<WWidget> text();
  std::unique_ptr<WWidget> grouping();
  std::unique_ptr<WWidget> layoutManagers();
  std::unique_ptr<WWidget> dialogs();
  std::unique_ptr<WWidget> images();
  std::unique_ptr<WWidget> css();
  std::unique_ptr<WWidget> themes();
  WWidget *loadingIndicator();
  void loadingIndicatorSelected(Wt::WString indicator);
};

#endif // LAYOUT_H_
