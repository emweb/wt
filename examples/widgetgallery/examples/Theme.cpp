#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(Theme)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WText *out = new Wt::WText(container);
out->setText("some text");

SAMPLE_END(return container)
