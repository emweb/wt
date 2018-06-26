#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButton)

auto result = Wt::cpp14::make_unique<Wt::WTemplate>();

result->setTemplateText("<div> ${pb1} ${pb2} </div>");

auto pb = Wt::cpp14::make_unique<Wt::WPushButton>("Click me!");
result->bindWidget("pb1", std::move(pb));  // By default the button is enabled.

pb = Wt::cpp14::make_unique<Wt::WPushButton>("Try to click me...");
auto pb_ = result->bindWidget("pb2", std::move(pb));
pb_->setEnabled(false);          // The second button is disabled.

SAMPLE_END(return std::move(result))
