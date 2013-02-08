#include <Wt/WPushButton>

SAMPLE_BEGIN(PushButtonLink)
Wt::WPushButton *button = new Wt::WPushButton("Navigate");
button->setLink(Wt::WLink(Wt::WLink::InternalPath, "/navigation/anchor"));

SAMPLE_END(return button)


