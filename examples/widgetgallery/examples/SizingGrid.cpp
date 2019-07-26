#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>

SAMPLE_BEGIN(SizingGrid)
auto parentContainer = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WLineEdit *edit = parentContainer->addNew<Wt::WLineEdit>();
edit->setPlaceholderText(".span8");
edit->setStyleClass("span8");

auto childContainer = Wt::cpp14::make_unique<Wt::WContainerWidget>();
childContainer->setStyleClass("controls-row");

edit = childContainer->addNew<Wt::WLineEdit>();
edit->setPlaceholderText(".span1");
edit->setStyleClass("span1");

edit = childContainer->addNew<Wt::WLineEdit>();
edit->setPlaceholderText(".span2");
edit->setStyleClass("span2");

edit = childContainer->addNew<Wt::WLineEdit>();
edit->setPlaceholderText(".span3");
edit->setStyleClass("span3");

edit = childContainer->addNew<Wt::WLineEdit>();
edit->setPlaceholderText(".span2");
edit->setStyleClass("span2");

parentContainer->addWidget(std::move(childContainer));

SAMPLE_END(return parentContainer)
