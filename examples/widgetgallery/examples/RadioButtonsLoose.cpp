#include <Wt/WContainerWidget.h>
#include <Wt/WRadioButton.h>

SAMPLE_BEGIN(RadioButtonsLoose)
auto container = std::make_unique<Wt::WContainerWidget>();

container->addNew<Wt::WRadioButton>("Radio me!");
container->addNew<Wt::WRadioButton>("Radio me too!");

SAMPLE_END(return std::move(container))
