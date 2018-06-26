#include <Wt/WPushButton.h>

SAMPLE_BEGIN(PushButtonLink)
auto button = Wt::cpp14::make_unique<Wt::WPushButton>("Navigate");
button->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/navigation/anchor"));

SAMPLE_END(return std::move(button))


