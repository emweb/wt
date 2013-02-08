#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WText>
#include <Wt/WVBoxLayout>

SAMPLE_BEGIN(NestedLayout)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->resize(200, 200);
container->setStyleClass("yellow-box centered");

Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
container->setLayout(vbox);

Wt::WText *item = new Wt::WText("Item 1");
item->setStyleClass("green-box");
vbox->addWidget(item, 1);

Wt::WHBoxLayout *hbox = new Wt::WHBoxLayout();
vbox->addLayout(hbox);

item = new Wt::WText("Item 2");
item->setStyleClass("green-box");
hbox->addWidget(item);

item = new Wt::WText("Item 3");
item->setStyleClass("blue-box");
hbox->addWidget(item);

SAMPLE_END(return container)
