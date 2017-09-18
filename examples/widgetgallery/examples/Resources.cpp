#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Resources)
auto container = cpp14::make_unique<WContainerWidget>();

WText *out = container->addWidget(cpp14::make_unique<WText>());
out->setText("<p>Resources</p>");

SAMPLE_END(return container)
