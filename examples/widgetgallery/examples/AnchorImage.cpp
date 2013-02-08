#include <Wt/WAnchor>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WLink>

SAMPLE_BEGIN(AnchorImage)
// Create an anchor that links to a URL through an image.
Wt::WAnchor *anchor = new Wt::WAnchor(Wt::WLink("http://www.emweb.be/"));
anchor->setTarget(Wt::TargetNewWindow);
new Wt::WImage(Wt::WLink("pics/emweb_small.jpg"), anchor);

SAMPLE_END(return anchor)
