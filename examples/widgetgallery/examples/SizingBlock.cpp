#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WString.h>
#include <Wt/WTextArea.h>

SAMPLE_BEGIN(SizingBlock)
auto container = cpp14::make_unique<WContainerWidget>();

WLineEdit *edit =
    container->addWidget(cpp14::make_unique<WLineEdit>());
edit->setStyleClass("input-block-level");
edit->setEmptyText("This is a line edit with style class"
                   " .input-block-level applied to it.");

WComboBox *combo =
    container->addWidget(cpp14::make_unique<WComboBox>());
combo->setStyleClass("input-block-level");
for (int i=1; i<5; ++i)
    combo->addItem("Combo box with style class .input-block-level - item"
                   + toString(i));

WTextArea *area =
    container->addWidget(cpp14::make_unique<WTextArea>());
area->setStyleClass("input-block-level");
area->setEmptyText("This is a text area with style class"
                   " .input-block-level applied to it.");

SAMPLE_END(return container)
