#include <Wt/WContainerWidget.h>
#include <Wt/WBreak.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Slider)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

container->addNew<Wt::WText>("In which year were you born?");
container->addNew<Wt::WBreak>();

Wt::WSlider *slider = container->addNew<Wt::WSlider>();
slider->resize(500, 50);
slider->setTickPosition(Wt::WSlider::TickPosition::TicksAbove);
slider->setTickInterval(10);
slider->setMinimum(1910);
slider->setMaximum(2010);
slider->setValue(1960);

container->addNew<Wt::WBreak>();
Wt::WText *out =
    container->addNew<Wt::WText>();

slider->valueChanged().connect([=] {
    out->setText("I was born in the year " + slider->valueText() + ".");
});

SAMPLE_END(return std::move(container))
