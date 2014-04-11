// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef PANEL_LIST_H_
#define PANEL_LIST_H_

#include <Wt/WContainerWidget>

namespace Wt {
  class WPanel;
}

class PanelList : public Wt::WContainerWidget
{
public:
  PanelList(Wt::WContainerWidget *parent);

  Wt::WPanel *addWidget(const Wt::WString& text, Wt::WWidget *w);
  void addPanel(Wt::WPanel *panel);
  void removePanel(Wt::WPanel *panel);

  using WContainerWidget::addWidget;

private:
  void onExpand(bool notUndo);

  int wasExpanded_;
};

#endif // PANEL_LIST_H_
