/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPanel.h>

#include "PanelList.h"

using namespace Wt;

PanelList::PanelList()
  : WContainerWidget()
{ }

WPanel *PanelList::addWidget(const WString& text, std::unique_ptr<WWidget> w)
{
  std::unique_ptr<WPanel> p
      = cpp14::make_unique<WPanel>();
  WPanel *result = p.get();
  p->setTitle(text);
  p->setCentralWidget(std::move(w));

  addPanel(std::move(p));

  return result;
}

void PanelList::addPanel(std::unique_ptr<WPanel> panel)
{
  panel->setCollapsible(true);
  panel->collapse();

  panel->expandedSS().connect(std::bind(&PanelList::onExpand, this, std::placeholders::_1, panel.get()));

  WContainerWidget::addWidget(std::move(panel));
}


void PanelList::onExpand(bool notUndo, WPanel *panel)
{
  if (notUndo) {
    wasExpanded_ = -1;

    for (unsigned i = 0; i < children().size(); ++i) {
      WPanel *p = dynamic_cast<WPanel *>(children()[i]);
      if (p != panel) {
        if (!p->isCollapsed())
          wasExpanded_ = i;
        p->collapse();
      }
    }
  } else {
    if (wasExpanded_ != -1) {
      WPanel *p = dynamic_cast<WPanel *>(children()[wasExpanded_]);
      p->expand();
    }
  }
}
