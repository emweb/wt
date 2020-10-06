#include <Wt/WLink.h>
#include <Wt/WPushButton.h>

SAMPLE_BEGIN(Path)
auto button = std::make_unique<Wt::WPushButton>("Next");
button->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/navigation/anchor"));

SAMPLE_END(return std::move(button))
