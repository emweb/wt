#include <Wt/WContainerWidget>
#include <Wt/WBorderLayout>
#include <Wt/WText>

SAMPLE_BEGIN(BorderLayout)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->setHeight(400);
container->setStyleClass("yellow-box");

Wt::WBorderLayout *layout = new Wt::WBorderLayout();
container->setLayout(layout);

const char *cell = "{1} item";

Wt::WText *item = new Wt::WText(Wt::WString(cell).arg("North"));
item->setStyleClass("green-box");
layout->addWidget(item, Wt::WBorderLayout::North);

item = new Wt::WText(Wt::WString(cell).arg("West"));
item->setStyleClass("green-box");
layout->addWidget(item, Wt::WBorderLayout::West);

item = new Wt::WText(Wt::WString(cell).arg("East"));
item->setStyleClass("green-box");
layout->addWidget(item, Wt::WBorderLayout::East);

item = new Wt::WText(Wt::WString(cell).arg("South"));
item->setStyleClass("green-box");
layout->addWidget(item, Wt::WBorderLayout::South);

item = new Wt::WText(Wt::WString(cell).arg("Center"));
item->setStyleClass("green-box");
layout->addWidget(item, Wt::WBorderLayout::Center);

SAMPLE_END(return container)
