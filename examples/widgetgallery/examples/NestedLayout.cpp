#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

SAMPLE_BEGIN(NestedLayout)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
container->resize(200, 200);
container->setStyleClass("yellow-box centered");

auto vbox = container->setLayout(Wt::cpp14::make_unique<Wt::WVBoxLayout>());

auto item = Wt::cpp14::make_unique<Wt::WText>("Item 1");
item->setStyleClass("green-box");
vbox->addWidget(std::move(item), 1);

auto hbox = vbox->addLayout(Wt::cpp14::make_unique<Wt::WHBoxLayout>());

item = Wt::cpp14::make_unique<Wt::WText>("Item 2");
item->setStyleClass("green-box");
hbox->addWidget(std::move(item));

item = Wt::cpp14::make_unique<Wt::WText>("Item 3");
item->setStyleClass("blue-box");
hbox->addWidget(std::move(item));

SAMPLE_END(return std::move(container))
