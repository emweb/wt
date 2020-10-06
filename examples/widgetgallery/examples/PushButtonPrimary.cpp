#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonPrimary)
auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WPushButton *button = container->addNew<Wt::WPushButton>("Save");
button->setStyleClass("btn-primary");

button = container->addNew<Wt::WPushButton>("Cancel");
button->setMargin(5, Wt::Side::Left);

SAMPLE_END(return std::move(container))
