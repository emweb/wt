#include <Wt/WContainerWidget.h>
#include <Wt/WInPlaceEdit.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(InPlaceEditButtons)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

auto ipe = container->addNew<Wt::WInPlaceEdit>("This is editable text");
ipe->setPlaceholderText("Enter something");

SAMPLE_END(return std::move(container))
