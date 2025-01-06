#include <Wt/WContainerWidget.h>
#include <Wt/WNotification.h>
#include <Wt/WPushButton.h>
#include <Wt/WString.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(NotificationSend)
auto container = std::make_unique<Wt::WContainerWidget>();

std::unique_ptr<Wt::WNotification> persistant = std::make_unique<Wt::WNotification>("You will see me everytime");
Wt::WNotification *p = persistant.get();
container->addChild(std::move(persistant));
p->setSilent();
Wt::WPushButton * button = container->addNew<Wt::WPushButton>("Receive notifications");

button->clicked().connect([=](){
    p->send();
    Wt::WNotification oneUse("You will only see me if you already accepted notifications.");
    oneUse.setSilent();
    oneUse.send();
});

SAMPLE_END(return std::move(container))
