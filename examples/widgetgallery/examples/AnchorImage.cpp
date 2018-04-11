#include <Wt/WAnchor>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WLink>

SAMPLE_BEGIN(AnchorImage)
// Create an anchor that links to a URL through an image.
Wt::WAnchor *anchor = new Wt::WAnchor(Wt::WLink("https://www.emweb.be/"));
anchor->setTarget(Wt::TargetNewWindow);
new Wt::WImage(Wt::WLink("https://www.emweb.be/css/emweb_small.png"), anchor);

SAMPLE_END(return anchor)
