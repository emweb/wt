#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WTextArea>

SAMPLE_BEGIN(SizingGrid)
Wt::WContainerWidget *parentContainer = new Wt::WContainerWidget();

Wt::WLineEdit *edit = new Wt::WLineEdit(parentContainer);
edit->setEmptyText(".span8");
edit->setStyleClass("span8");

Wt::WContainerWidget *childContainer = new Wt::WContainerWidget();
childContainer->setStyleClass("controls-row");

edit = new Wt::WLineEdit(childContainer);
edit->setEmptyText(".span1");
edit->setStyleClass("span1");

edit = new Wt::WLineEdit(childContainer);
edit->setEmptyText(".span2");
edit->setStyleClass("span2");

edit = new Wt::WLineEdit(childContainer);
edit->setEmptyText(".span3");
edit->setStyleClass("span3");

edit = new Wt::WLineEdit(childContainer);
edit->setEmptyText(".span2");
edit->setStyleClass("span2");

parentContainer->addWidget(childContainer);

SAMPLE_END(return parentContainer)
