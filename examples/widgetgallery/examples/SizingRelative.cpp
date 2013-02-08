#include <Wt/WBreak>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WTextArea>

SAMPLE_BEGIN(SizingRelative)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WLineEdit *edit = new Wt::WLineEdit(container);
edit->setEmptyText(".input-mini");
edit->setStyleClass("input-mini");
new Wt::WBreak(container);
edit = new Wt::WLineEdit(container);
edit->setEmptyText(".input-small");
edit->setStyleClass("input-small");
new Wt::WBreak(container);
edit = new Wt::WLineEdit(container);
edit->setEmptyText(".input-medium");
edit->setStyleClass("input-medium");
new Wt::WBreak(container);
edit = new Wt::WLineEdit(container);
edit->setEmptyText(".input-large");
edit->setStyleClass("input-large");
new Wt::WBreak(container);
edit = new Wt::WLineEdit(container);
edit->setEmptyText(".input-xlarge");
edit->setStyleClass("input-xlarge");
new Wt::WBreak(container);
edit = new Wt::WLineEdit(container);
edit->setEmptyText(".input-xxlarge");
edit->setStyleClass("input-xxlarge");

SAMPLE_END(return container)
