#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WText.h>
#include <memory>

SAMPLE_BEGIN(HBoxLayoutStretch)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
container->setStyleClass("yellow-box");

auto hbox = container->setLayout(Wt::cpp14::make_unique<Wt::WHBoxLayout>());

auto item = Wt::cpp14::make_unique<Wt::WText>("Item 1");
item->setStyleClass("green-box");
hbox->addWidget(std::move(item), 1);
  
item = Wt::cpp14::make_unique<Wt::WText>("Item 2");
item->setStyleClass("blue-box");
hbox->addWidget(std::move(item));

SAMPLE_END(return std::move(container))
