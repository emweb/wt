#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SliderNative)
auto container = std::make_unique<Wt::WContainerWidget>();

container->addNew<Wt::WText>("What is your favorite odd number?");
container->addNew<Wt::WBreak>();

Wt::WSlider *slider = container->addNew<Wt::WSlider>();
slider->resize(300, 50);
slider->setNativeControl(true);
slider->setTickInterval(10);
slider->setRange(1, 99);
slider->setStep(2);

container->addNew<Wt::WBreak>();
Wt::WText *out = container->addNew<Wt::WText>();
out->setMargin(10, Wt::Side::Left);

slider->valueChanged().connect([=] {
    out->setText("So your favorite odd number is " +
                 slider->valueText() + " !");
});

SAMPLE_END(return std::move(container))
