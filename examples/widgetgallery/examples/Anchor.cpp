#include <Wt/WAnchor>
#include <Wt/WContainerWidget>
#include <Wt/WLink>

SAMPLE_BEGIN(Anchor)
// Create an anchor that links to a URL through clickable text.
Wt::WAnchor *anchor =
        new Wt::WAnchor(Wt::WLink("http://www.webtoolkit.eu/"),
                        "Wt homepage (in a new window)");
anchor->setTarget(Wt::TargetNewWindow);

SAMPLE_END(return anchor)
