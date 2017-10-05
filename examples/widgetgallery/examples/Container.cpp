#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Container)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

container->addWidget(Wt::cpp14::make_unique<Wt::WText>("A first widget"));

for (unsigned int i = 0; i < 3; ++i) {
    // A widget can be added to a container by using addWidget()
    container->addWidget(Wt::cpp14::make_unique<Wt::WText>(Wt::WString("<p>Text {1}</p>").arg(i)));
}

SAMPLE_END(return std::move(container))
