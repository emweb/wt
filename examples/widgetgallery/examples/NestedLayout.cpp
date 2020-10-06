#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

SAMPLE_BEGIN(NestedLayout)
auto container = std::make_unique<Wt::WContainerWidget>();
container->resize(200, 200);
container->setStyleClass("yellow-box centered");

auto vbox = container->setLayout(std::make_unique<Wt::WVBoxLayout>());

auto item = std::make_unique<Wt::WText>("Item 1");
item->setStyleClass("green-box");
vbox->addWidget(std::move(item), 1);

#ifndef WT_TARGET_JAVA
auto hbox = vbox->addLayout(std::make_unique<Wt::WHBoxLayout>());
#else // WT_TARGET_JAVA
auto hbox = std::make_unique<Wt::WHBoxLayout>();
vbox->addLayout(hbox);
#endif // WT_TARGET_JAVA

item = std::make_unique<Wt::WText>("Item 2");
item->setStyleClass("green-box");
hbox->addWidget(std::move(item));

item = std::make_unique<Wt::WText>("Item 3");
item->setStyleClass("blue-box");
hbox->addWidget(std::move(item));

SAMPLE_END(return std::move(container))
