#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SliderSteps)
auto container = std::make_unique<Wt::WContainerWidget>();

container->addNew<Wt::WText>("Try to select '7'. I bet you can't.");
container->addNew<Wt::WBreak>();

Wt::WSlider *slider = container->addNew<Wt::WSlider>();
slider->resize(300, 50);
slider->setTickPosition(Wt::WSlider::TicksBothSides);
slider->setRange(0, 10);
slider->setStep(3);

container->addNew<Wt::WBreak>();
Wt::WText *out = container->addNew<Wt::WText>();
out->setMargin(10, Wt::Side::Left);

slider->valueChanged().connect([=] {
    out->setText("That's a failure, you selected: '" +
                 slider->valueText() + "'!");
});

SAMPLE_END(return std::move(container))
