#include <Wt/WContainerWidget.h>
#include <Wt/WNotification.h>
#include <Wt/WPushButton.h>
#include <Wt/WString.h>
#include <Wt/WText.h>


class Channel: public Wt::WContainerWidget
{
public:
    Channel()
        : newMsg_(0)
    {
#ifndef WT_TARGET_JAVA
        notif_.setIcon("icons/wt.png");
#else
        notif_.setIcon("icons/jwt.png");
#endif
        notif_.setSilent();
        notif_.clicked().connect(this, &Channel::onNotifClick);
    }

    void newMsgSent()
    {
        ++newMsg_;
        notif_.setTitle(std::to_string(newMsg_)+" new messages");
        notif_.setBody("There are "+std::to_string(newMsg_)+" new messages waiting for you.");
        notif_.send();
    }

private:
    int newMsg_;
    Wt::WNotification notif_;

    void onNotifClick() 
    {
        newMsg_ = 0;
    }
};

SAMPLE_BEGIN(NotificationChat)
auto container = std::make_unique<Wt::WContainerWidget>();
Channel *channel = container->addWidget(std::make_unique<Channel>());

Wt::WPushButton * button = channel->addNew<Wt::WPushButton>("Send Message");

button->clicked().connect([=](){
    channel->newMsgSent();
});

SAMPLE_END(return std::move(container))