#include <Wt/WButtonGroup>
#include <Wt/WContainerWidget>
#include <Wt/WRadioButton>

SAMPLE_BEGIN(RadioButtonStack)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WButtonGroup *group = new Wt::WButtonGroup(container);
Wt::WRadioButton *button;

button = new Wt::WRadioButton("Radio me!", container);
button->setInline(false);
group->addButton(button);

button = new Wt::WRadioButton("No, radio me!", container);
button->setInline(false);
group->addButton(button);

button = new Wt::WRadioButton("Nono, radio me!", container);
button->setInline(false);
group->addButton(button);

group->setSelectedButtonIndex(0); // Select the first button by default.

SAMPLE_END(return container)
