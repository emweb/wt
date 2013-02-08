#include <Wt/WContainerWidget>
#include <Wt/WRadioButton>

SAMPLE_BEGIN(RadioButtonsLoose)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new Wt::WRadioButton("Radio me!", container);
new Wt::WRadioButton("Radio me too!", container);

SAMPLE_END(return container)
