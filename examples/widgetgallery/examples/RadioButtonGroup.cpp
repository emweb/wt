#include <Wt/WButtonGroup.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WRadioButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(RadioButtonGroup)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

auto group = std::make_shared<Wt::WButtonGroup>();
Wt::WRadioButton *button;

button = container->addNew<Wt::WRadioButton>("Radio me!");
group->addButton(button);

button = container->addNew<Wt::WRadioButton>("No, radio me!");
group->addButton(button);

button = container->addNew<Wt::WRadioButton>("Nono, radio me!");
group->addButton(button);

group->setSelectedButtonIndex(0); // Select the first button by default.

SAMPLE_END(return std::move(container))
