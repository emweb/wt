#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(Container)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

container->addWidget(new Wt::WText("A first widget"));

for (unsigned int i = 0; i < 3; ++i) {
    // A widget can be added to a container by passing the container as
    // the last constructor argument.
    new Wt::WText(Wt::WString::fromUTF8("<p>Text {1}</p>").arg(i), container);
}

SAMPLE_END(return container)
