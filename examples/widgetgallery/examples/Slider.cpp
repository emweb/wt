#include <Wt/WContainerWidget.h>
#include <Wt/WBreak.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Slider)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

container->addWidget(Wt::cpp14::make_unique<Wt::WText>("In which year were you born?"));
container->addWidget(Wt::cpp14::make_unique<Wt::WBreak>());

Wt::WSlider *slider =
    container->addWidget(Wt::cpp14::make_unique<Wt::WSlider>());
slider->resize(500, 50);
slider->setTickPosition(Wt::WSlider::TickPosition::TicksAbove);
slider->setTickInterval(10);
slider->setMinimum(1910);
slider->setMaximum(2010);
slider->setValue(1960);

container->addWidget(Wt::cpp14::make_unique<Wt::WBreak>());
Wt::WText *out =
    container->addWidget(Wt::cpp14::make_unique<Wt::WText>());

slider->valueChanged().connect([=] {
    out->setText("I was born in the year " + slider->valueText() + ".");
});

SAMPLE_END(return std::move(container))
