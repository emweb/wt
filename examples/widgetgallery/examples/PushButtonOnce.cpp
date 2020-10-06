#include <Wt/WPushButton.h>

SAMPLE_BEGIN(PushButtonOnce)
auto okPtr = std::make_unique<Wt::WPushButton>("Send");
auto ok = okPtr.get();

ok->clicked().connect(ok, &Wt::WPushButton::disable);
ok->clicked().connect([=] {
    ok->setText("Thank you");
});

SAMPLE_END(return std::move(okPtr))
