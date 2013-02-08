#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WText>

SAMPLE_BEGIN(HBoxLayout)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->setStyleClass("yellow-box");

Wt::WHBoxLayout *hbox = new Wt::WHBoxLayout();
container->setLayout(hbox);

Wt::WText *item = new Wt::WText("Item 1");
item->setStyleClass("green-box");
hbox->addWidget(item);
  
item = new Wt::WText("Item 2");
item->setStyleClass("blue-box");
hbox->addWidget(item);

SAMPLE_END(return container)
