#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WString.h>
#include <Wt/WTextArea.h>

SAMPLE_BEGIN(SizingBlock)
auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WLineEdit *edit = container->addNew<Wt::WLineEdit>();
edit->setStyleClass("input-block-level");
edit->setPlaceholderText("This is a line edit with style class"
                   " .input-block-level applied to it.");

Wt::WComboBox *combo = container->addNew<Wt::WComboBox>();
combo->setStyleClass("input-block-level");
for (int i=1; i<5; ++i)
    combo->addItem("Combo box with style class .input-block-level - item"
                   + std::to_string(i));

Wt::WTextArea *area = container->addNew<Wt::WTextArea>();
area->setStyleClass("input-block-level");
area->setPlaceholderText("This is a text area with style class"
                   " .input-block-level applied to it.");

SAMPLE_END(return container)
