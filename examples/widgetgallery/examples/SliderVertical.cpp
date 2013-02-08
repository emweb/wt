#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WSlider>
#include <Wt/WText>

SAMPLE_BEGIN(SliderVertical)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new Wt::WText("How much does Wt increase your efficiency?", container);
new Wt::WBreak(container);

Wt::WSlider *verticalSlider = new Wt::WSlider(Wt::Vertical, container);
verticalSlider->resize(50, 150);
verticalSlider->setTickPosition(Wt::WSlider::TicksBothSides);
verticalSlider->setRange(5, 50);

new Wt::WBreak(container);
Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Left);

verticalSlider->valueChanged().connect(std::bind([=] () {
    out->setText("Currenly, my efficiency increased " +
		 verticalSlider->valueText() + "%!");
}));

SAMPLE_END(return container)
