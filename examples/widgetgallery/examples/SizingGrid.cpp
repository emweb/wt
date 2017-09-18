#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>

SAMPLE_BEGIN(SizingGrid)
auto parentContainer = cpp14::make_unique<WContainerWidget>();

WLineEdit *edit =
    parentContainer->addWidget(cpp14::make_unique<WLineEdit>());
edit->setEmptyText(".span8");
edit->setStyleClass("span8");

auto childContainer = cpp14::make_unique<WContainerWidget>();
childContainer->setStyleClass("controls-row");

edit =
    childContainer->addWidget(cpp14::make_unique<WLineEdit>());
edit->setEmptyText(".span1");
edit->setStyleClass("span1");

edit =
    childContainer->addWidget(cpp14::make_unique<WLineEdit>());
edit->setEmptyText(".span2");
edit->setStyleClass("span2");

edit =
    childContainer->addWidget(cpp14::make_unique<WLineEdit>());
edit->setEmptyText(".span3");
edit->setStyleClass("span3");

edit =
    childContainer->addWidget(cpp14::make_unique<WLineEdit>());
edit->setEmptyText(".span2");
edit->setStyleClass("span2");

parentContainer->addWidget(std::move(childContainer));

SAMPLE_END(return parentContainer)
