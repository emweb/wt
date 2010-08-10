/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPanel>

#include "PanelList.h"

using namespace Wt;

PanelList::PanelList(WContainerWidget *parent)
  : WContainerWidget(parent)
{ }

WPanel *PanelList::addWidget(const WString& text, WWidget *w)
{
  WPanel *p = new WPanel();
  p->setTitle(text);
  p->setCentralWidget(w);

  addPanel(p);

  return p;
}

void PanelList::addPanel(WPanel *panel)
{
  panel->setCollapsible(true);
  panel->collapse();

  panel->expandedSS().connect(this, &PanelList::onExpand);

  WContainerWidget::addWidget(panel);
}

void PanelList::onExpand(bool notUndo)
{
  WPanel *panel = dynamic_cast<WPanel *>(sender());

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
