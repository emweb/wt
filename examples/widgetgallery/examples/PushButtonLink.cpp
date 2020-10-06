#include <Wt/WPushButton.h>

SAMPLE_BEGIN(PushButtonLink)
auto button = std::make_unique<Wt::WPushButton>("Navigate");
button->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/navigation/anchor"));

SAMPLE_END(return std::move(button))


