#include <Wt/WAnchor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WLink.h>

SAMPLE_BEGIN(AnchorImage)

// Create an anchor that links to a URL through an image.
Wt::WLink link = Wt::WLink("http://www.emweb.be/");
link.setTarget(Wt::LinkTarget::NewWindow);

std::unique_ptr<Wt::WAnchor> anchor = Wt::cpp14::make_unique<Wt::WAnchor>(link);
anchor->addWidget(Wt::cpp14::make_unique<Wt::WImage>(Wt::WLink("pics/emweb_small.jpg")));

SAMPLE_END(return std::move(anchor))
