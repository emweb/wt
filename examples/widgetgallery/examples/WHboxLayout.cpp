#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(HBoxLayout)

auto container = cpp14::make_unique<WContainerWidget>();

container->addWidget(cpp14::make_unique<WText>("<p>A first widget</p>"));

for (unsigned i = 0; i < 3; ++i) {
    // a widget can be added to a container by passing the container as
    // the last constructor argument
    container->addWidget(cpp14::make_unique<WText>("<p>Text " + asString(i) + "</p>");
}

SAMPLE_END(return container)
