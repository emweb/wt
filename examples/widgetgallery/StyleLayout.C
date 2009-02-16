/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "StyleLayout.h"
#include "EventDisplayer.h"

#include <Wt/WBorderLayout>
#include <Wt/WGridLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>

#include <Wt/WText>

using namespace Wt;

StyleLayout::StyleLayout(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  new WText(tr("style-layout-intro"), this);
}

void StyleLayout::populateSubMenu(WMenu *menu)
{
  menu->addItem("CSS", css());
  menu->addItem("WBoxLayout", wBoxLayout());
  menu->addItem("WGridLayout", wGridLayout());
  menu->addItem("WBorderLayout", wBorderLayout());
}

WWidget *StyleLayout::css()
{
  return new WText(tr("style-and-layout-css"));
}

WWidget *StyleLayout::wBoxLayout()
{
  WContainerWidget *result = new WContainerWidget();
  topic("WHBoxLayout", "WVBoxLayout", result);

  new WText(tr("layout-WBoxLayout"), result);

  WContainerWidget *container;
  WText *item;
  WHBoxLayout *hbox;
  WVBoxLayout *vbox;

  /*
   * first hbox
   */
  container = new WContainerWidget(result);
  container->setStyleClass("yellow-box");
  hbox = new WHBoxLayout();
  container->setLayout(hbox);

  item = new WText(tr("layout-item1"));
  item->setStyleClass("green-box");
  hbox->addWidget(item);
  
  item = new WText(tr("layout-item2"));
  item->setStyleClass("blue-box");
  hbox->addWidget(item);

  new WText(tr("layout-WBoxLayout-stretch"), result);

  /*
   * second hbox
   */
  container = new WContainerWidget(result);
  container->setStyleClass("yellow-box");
  hbox = new WHBoxLayout();
  container->setLayout(hbox);

  item = new WText(tr("layout-item1"));
  item->setStyleClass("green-box");
  hbox->addWidget(item, 1);
  
  item = new WText(tr("layout-item2"));
  item->setStyleClass("blue-box");
  hbox->addWidget(item);

  new WText(tr("layout-WBoxLayout-vbox"), result);

  /*
   * first vbox
   */
  container = new WContainerWidget(result);
  container->resize(150, 150);
  container->setStyleClass("yellow-box centered");
  vbox = new WVBoxLayout();
  container->setLayout(vbox);

  item = new WText(tr("layout-item1"));
  item->setStyleClass("green-box");
  vbox->addWidget(item);
  
  item = new WText(tr("layout-item2"));
  item->setStyleClass("blue-box");
  vbox->addWidget(item);

  /*
   * second vbox
   */
  container = new WContainerWidget(result);
  container->resize(150, 150);
  container->setStyleClass("yellow-box centered");
  vbox = new WVBoxLayout();
  container->setLayout(vbox);

  item = new WText(tr("layout-item1"));
  item->setStyleClass("green-box");
  vbox->addWidget(item, 1);
  
  item = new WText(tr("layout-item2"));
  item->setStyleClass("blue-box");
  vbox->addWidget(item);

  new WText(tr("layout-WBoxLayout-nested"), result);

  /*
   * nested boxes
   */
  container = new WContainerWidget(result);
  container->resize(200, 200);
  container->setStyleClass("yellow-box centered");

  vbox = new WVBoxLayout();
  container->setLayout(vbox);

  item = new WText(tr("layout-item1"));
  item->setStyleClass("green-box");
  vbox->addWidget(item, 1);

  hbox = new WHBoxLayout();
  vbox->addLayout(hbox);

  item = new WText(tr("layout-item2"));
  item->setStyleClass("green-box");
  hbox->addWidget(item);

  item = new WText(tr("layout-item3"));
  item->setStyleClass("blue-box");
  hbox->addWidget(item);

  return result;
}

WWidget *StyleLayout::wGridLayout()
{
  WContainerWidget *result = new WContainerWidget();
  topic("WGridLayout", result);

  new WText(tr("layout-WGridLayout"), result);

  WContainerWidget *container;

  container = new WContainerWidget(result);
  container->resize(WLength(), 400);
  container->setStyleClass("yellow-box");
  WGridLayout *grid = new WGridLayout();
  container->setLayout(grid);

  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 4; ++column) {
      WText *t = new WText(tr("grid-item").arg(row).arg(column));
      if (row == 1 || column == 1 || column == 2)
	t->setStyleClass("blue-box");
      else
	t->setStyleClass("green-box");
      grid->addWidget(t, row, column);
    }
  }

  grid->setRowStretch(1, 1);
  grid->setColumnStretch(1, 1);
  grid->setColumnStretch(2, 1);

  return result;
}

WWidget *StyleLayout::wBorderLayout()
{
  WContainerWidget *result = new WContainerWidget();
  topic("WBorderLayout", result);

  new WText(tr("layout-WBorderLayout"), result);

  WContainerWidget *container;

  container = new WContainerWidget(result);
  container->resize(WLength(), 400);
  container->setStyleClass("yellow-box");
  WBorderLayout *layout = new WBorderLayout();
  container->setLayout(layout);

  WText *item;

  item = new WText(tr("borderlayout-item").arg("North"));
  item->setStyleClass("green-box");
  layout->addWidget(item, WBorderLayout::North);

  item = new WText(tr("borderlayout-item").arg("West"));
  item->setStyleClass("green-box");
  layout->addWidget(item, WBorderLayout::West);

  item = new WText(tr("borderlayout-item").arg("East"));
  item->setStyleClass("green-box");
  layout->addWidget(item, WBorderLayout::East);

  item = new WText(tr("borderlayout-item").arg("South"));
  item->setStyleClass("green-box");
  layout->addWidget(item, WBorderLayout::South);

  item = new WText(tr("borderlayout-item").arg("Center"));
  item->setStyleClass("green-box");
  layout->addWidget(item, WBorderLayout::Center);

  return result;
}
