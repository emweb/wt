// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef PANEL_LIST_H_
#define PANEL_LIST_H_

#include <Wt/WContainerWidget.h>

namespace Wt {
  class WPanel;
}

class PanelList : public Wt::WContainerWidget
{
public:
  PanelList();

  Wt::WPanel *addWidget(const Wt::WString& text, std::unique_ptr<Wt::WWidget> w);
  void addPanel(std::unique_ptr<Wt::WPanel> panel);
  void removePanel(Wt::WPanel *panel);

  using WContainerWidget::addWidget;

private:
  void onExpand(bool notUndo, Wt::WPanel *panel);
  //void onExpand(bool notUndo);

  int wasExpanded_;
};

#endif // PANEL_LIST_H_
