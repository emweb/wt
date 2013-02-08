#include <Wt/WPushButton>

SAMPLE_BEGIN(PushButtonOnce)
Wt::WPushButton *ok = new Wt::WPushButton("Send");

ok->clicked().connect(ok, &Wt::WPushButton::disable);
ok->clicked().connect(std::bind([=]() {
    ok->setText("Thank you");
}));

SAMPLE_END(return ok)
