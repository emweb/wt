#include <Wt/WContainerWidget.h>
#include <Wt/WNotification.h>
#include <Wt/WPushButton.h>
#include <Wt/WString.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(AskNotificationPermission)
auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WPushButton * button = container->addNew<Wt::WPushButton>("Get asked for notification");
Wt::WText *text = container->addNew<Wt::WText>();

button->clicked().connect([=](){
    Wt::WNotification::askPermission();
    button->setEnabled(false);
});

Wt::WNotification::permissionUpdated().connect([=](Wt::WNotification::Permission permission){
    if (permission == Wt::WNotification::Permission::Granted) {
        text->setText("You have allowed us to send you notifications.");
    } else {
        text->setText("You have not allowed us to send you notifications.");
    }
});

SAMPLE_END(return std::move(container))
