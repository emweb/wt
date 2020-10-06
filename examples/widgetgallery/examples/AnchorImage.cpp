#include <Wt/WAnchor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WLink.h>

SAMPLE_BEGIN(AnchorImage)

// Create an anchor that links to a URL through an image.
Wt::WLink link = Wt::WLink("https://www.emweb.be/");
link.setTarget(Wt::LinkTarget::NewWindow);

std::unique_ptr<Wt::WAnchor> anchor = std::make_unique<Wt::WAnchor>(link);
anchor->addNew<Wt::WImage>(Wt::WLink("https://www.emweb.be/css/emweb_small.png"));

SAMPLE_END(return std::move(anchor))
