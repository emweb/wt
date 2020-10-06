#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WText.h>

#include <string>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(HBoxLayout)

auto container = std::make_unique<WContainerWidget>();

container->addWidget(std::make_unique<WText>("<p>A first widget</p>"));

for (unsigned i = 0; i < 3; ++i) {
    // a widget can be added to a container by passing the container as
    // the last constructor argument
    container->addWidget(std::make_unique<WText>("<p>Text " + std::to_string(i) + "</p>"));
}

SAMPLE_END(return container)
