#include <Wt/WContainerWidget.h>
#include <Wt/WNotification.h>
#include <Wt/WPushButton.h>
#include <Wt/WString.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(NotificationChat)

class Channel: public Wt::WContainerWidget
{
public:
    Channel()
        : newMsg_(0)
    {
        notif_.setIcon("icons/wt.png");
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

auto container = std::make_unique<Channel>();
auto channel = container.get();

Wt::WPushButton * button = channel->addNew<Wt::WPushButton>("Send Message");

button->clicked().connect([=](){
    channel->newMsgSent();
});

SAMPLE_END(return std::move(container))