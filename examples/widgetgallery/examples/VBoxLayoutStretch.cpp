#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

SAMPLE_BEGIN(VBoxLayoutStretch)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
container->resize(150, 150);
container->setStyleClass("yellow-box centered");

auto vbox = container->setLayout(Wt::cpp14::make_unique<Wt::WVBoxLayout>());

auto item = Wt::cpp14::make_unique<Wt::WText>("Item 1");
item->setStyleClass("green-box");
vbox->addWidget(std::move(item), 1);
  
item = Wt::cpp14::make_unique<Wt::WText>("Item 2");
item->setStyleClass("blue-box");
vbox->addWidget(std::move(item));

SAMPLE_END(return std::move(container))
