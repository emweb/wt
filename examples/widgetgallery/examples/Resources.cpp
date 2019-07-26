#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Resources)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WText *out = container->addNew<Wt::WText>();
out->setText("<p>Resources</p>");

SAMPLE_END(return container)
