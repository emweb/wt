#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WVBoxLayout>

SAMPLE_BEGIN(VBoxLayout)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->resize(150, 150);
container->setStyleClass("yellow-box centered");

Wt::WVBoxLayout *vbox = new Wt::WVBoxLayout();
container->setLayout(vbox);

Wt::WText *item = new Wt::WText("Item 1");
item->setStyleClass("green-box");
vbox->addWidget(item);
  
item = new Wt::WText("Item 2");
item->setStyleClass("blue-box");
vbox->addWidget(item);

SAMPLE_END(return container)
