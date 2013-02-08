#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WTemplate>

SAMPLE_BEGIN(PushButtonPrimary)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WPushButton *button = new Wt::WPushButton("Save", container);
button->setStyleClass("btn-primary");

button = new Wt::WPushButton("Cancel", container);
button->setMargin(5, Wt::Left);

SAMPLE_END(return container)
