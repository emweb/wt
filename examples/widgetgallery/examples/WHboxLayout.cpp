#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WText>

SAMPLE_BEGIN(HBoxLayout)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

container->addWidget(new Wt::WText("<p>A first widget</p>"));

for (unsigned i = 0; i < 3; ++i) {
    // a widget can be added to a container by passing the container as
    // the last constructor argument
    new Wt::WText("<p>Text " + boost::lexical_cast<std::string>(i) + "</p>",
		  container);
}

SAMPLE_END(return container)
