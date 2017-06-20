#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WLink.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Image)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WImage *image =
    container->addWidget(Wt::cpp14::make_unique<Wt::WImage>(Wt::WLink("icons/wt_powered.jpg")));
image->setAlternateText("Wt logo");

Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());
out->setMargin(10, Wt::Side::Left);

image->clicked().connect([=] (const Wt::WMouseEvent& e) {
    out->setText("You clicked the Wt logo at "
                 "(" + std::to_string(e.widget().x) +
                 "," + std::to_string(e.widget().y) +
		 ").");
});

SAMPLE_END(return std::move(container))
