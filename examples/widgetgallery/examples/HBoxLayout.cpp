#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(HBoxLayout)

auto container = std::make_unique<Wt::WContainerWidget>();
container->setStyleClass("yellow-box");

auto hbox = container->setLayout(std::make_unique<Wt::WHBoxLayout>());

std::unique_ptr<Wt::WText> item = std::make_unique<Wt::WText>("Item 1");
item->setStyleClass("green-box");
hbox->addWidget(std::move(item));

item = std::make_unique<Wt::WText>("Item 2");
item->setStyleClass("blue-box");
hbox->addWidget(std::move(item));

SAMPLE_END(return std::move(container))
