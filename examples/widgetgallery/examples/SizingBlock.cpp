#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WString>
#include <Wt/WTextArea>

SAMPLE_BEGIN(SizingBlock)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WLineEdit *edit = new Wt::WLineEdit(container);
edit->setStyleClass("input-block-level");
edit->setEmptyText("This is a line edit with style class"
                   " .input-block-level applied to it.");

Wt::WComboBox *combo = new Wt::WComboBox(container);
combo->setStyleClass("input-block-level");
for (int i=1; i<5; ++i)
    combo->addItem("Combo box with style class .input-block-level - item"
                   + boost::lexical_cast<std::string>(i));

Wt::WTextArea *area = new Wt::WTextArea(container);
area->setStyleClass("input-block-level");
area->setEmptyText("This is a text area with style class"
                   " .input-block-level applied to it.");

SAMPLE_END(return container)
