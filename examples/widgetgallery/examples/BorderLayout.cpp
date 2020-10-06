#include <Wt/WContainerWidget.h>
#include <Wt/WBorderLayout.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(BorderLayout)

auto container = std::make_unique<Wt::WContainerWidget>();
container->setHeight(400);
container->setStyleClass("yellow-box");

auto layout = container->setLayout(std::make_unique<Wt::WBorderLayout>());

const char *cell = "{1} item";

auto item = std::make_unique<Wt::WText>(Wt::WString(cell).arg("North"));
item->setStyleClass("green-box");
layout->addWidget(std::move(item), Wt::LayoutPosition::North);

item = std::make_unique<Wt::WText>(Wt::WString(cell).arg("West"));
item->setStyleClass("green-box");
layout->addWidget(std::move(item), Wt::LayoutPosition::West);

item = std::make_unique<Wt::WText>(Wt::WString(cell).arg("East"));
item->setStyleClass("green-box");
layout->addWidget(std::move(item), Wt::LayoutPosition::East);

item = std::make_unique<Wt::WText>(Wt::WString(cell).arg("South"));
item->setStyleClass("green-box");
layout->addWidget(std::move(item), Wt::LayoutPosition::South);

item = std::make_unique<Wt::WText>(Wt::WString(cell).arg("Center"));
item->setStyleClass("green-box");
layout->addWidget(std::move(item), Wt::LayoutPosition::Center);

SAMPLE_END(return std::move(container))
