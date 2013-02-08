#include <Wt/WLink>
#include <Wt/WPushButton>

SAMPLE_BEGIN(Path)
Wt::WPushButton *button = new Wt::WPushButton("Next");
button->setLink(Wt::WLink(Wt::WLink::InternalPath, "/navigation/anchor"));

SAMPLE_END(return button)
