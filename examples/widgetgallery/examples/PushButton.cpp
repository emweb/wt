#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButton)

auto result = Wt::cpp14::make_unique<Wt::WTemplate>();

result->setTemplateText("<div> ${pb1} ${pb2} </div>");

auto pb = result->bindWidget("pb1", Wt::cpp14::make_unique<Wt::WPushButton>("Click me!"));  // By default the button is enabled.

pb = result->bindWidget("pb2", Wt::cpp14::make_unique<Wt::WPushButton>("Try to click me..."));
pb->setEnabled(false);          // The second button is disabled.

SAMPLE_END(return std::move(result))
