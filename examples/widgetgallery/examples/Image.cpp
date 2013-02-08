#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WLink>
#include <Wt/WText>

SAMPLE_BEGIN(Image)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WImage *image = new Wt::WImage(Wt::WLink("icons/wt_powered.jpg"),
                                   container);
image->setAlternateText("Wt logo");

Wt::WText *out = new Wt::WText(container);
out->setMargin(10, Wt::Left);

image->clicked().connect(std::bind([=] (const Wt::WMouseEvent& e) {
    out->setText("You clicked the Wt logo at "
		 "(" + boost::lexical_cast<std::string>(e.widget().x) +
		 "," + boost::lexical_cast<std::string>(e.widget().y) +
		 ").");
}, std::placeholders::_1));

SAMPLE_END(return container)
