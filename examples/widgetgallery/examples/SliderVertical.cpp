#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SliderVertical)
auto container = std::make_unique<Wt::WContainerWidget>();

container->addNew<Wt::WText>("How much does Wt increase your efficiency?");
container->addNew<Wt::WBreak>();

Wt::WSlider *verticalSlider = container->addNew<Wt::WSlider>(Wt::Orientation::Vertical);
verticalSlider->resize(50, 150);
verticalSlider->setTickPosition(Wt::WSlider::TicksBothSides);
verticalSlider->setRange(5, 50);

container->addNew<Wt::WBreak>();
Wt::WText *out = container->addNew<Wt::WText>();
out->setMargin(10, Wt::Side::Left);

verticalSlider->valueChanged().connect([=] {
    out->setText("Currenly, my efficiency increased " +
		 verticalSlider->valueText() + "%!");
});

SAMPLE_END(return std::move(container))
