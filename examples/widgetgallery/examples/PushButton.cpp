#include <Wt/WPushButton>
#include <Wt/WTemplate>

SAMPLE_BEGIN(PushButton)

Wt::WTemplate *result = new Wt::WTemplate();

result->setTemplateText("<div> ${pb1} ${pb2} </div>");

Wt::WPushButton *pb = new Wt::WPushButton("Click me!");
result->bindWidget("pb1", pb);  // By default the button is enabled.

pb = new Wt::WPushButton("Try to click me...");
result->bindWidget("pb2", pb);
pb->setEnabled(false);          // The second button is disabled.

SAMPLE_END(return result)
