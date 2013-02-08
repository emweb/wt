#include <Wt/WContainerWidget>
#include <Wt/WBreak>
#include <Wt/WSlider>
#include <Wt/WText>

SAMPLE_BEGIN(Slider)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new Wt::WText("In which year are you born?", container);
new Wt::WBreak(container);

Wt::WSlider *slider = new Wt::WSlider(container);
slider->resize(500, 50);
slider->setTickPosition(Wt::WSlider::TicksAbove);
slider->setTickInterval(10);
slider->setMinimum(1910);
slider->setMaximum(2010);
slider->setValue(1960);

new Wt::WBreak(container);
Wt::WText *out = new Wt::WText(container);

slider->valueChanged().connect(std::bind([=] () {
    out->setText("I'm born in the year " + slider->valueText() + ".");
}));

SAMPLE_END(return container)
