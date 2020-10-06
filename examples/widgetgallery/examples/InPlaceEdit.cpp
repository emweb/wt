#include <Wt/WContainerWidget.h>
#include <Wt/WInPlaceEdit.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(InPlaceEdit)

auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WInPlaceEdit *ipe = container->addNew<Wt::WInPlaceEdit>("This is editable text");
ipe->setPlaceholderText("Enter something");
ipe->setButtonsEnabled(false);

SAMPLE_END(return std::move(container))
