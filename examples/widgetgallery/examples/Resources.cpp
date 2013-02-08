#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(Resources)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WText *out = new Wt::WText(container);
out->setText("<p>Resources</p>");

SAMPLE_END(return container)
