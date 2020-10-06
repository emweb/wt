#include <Wt/WAnchor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLink.h>

SAMPLE_BEGIN(Anchor)

// Create an anchor that links to a URL through clickable text.
Wt::WLink link = Wt::WLink("https://www.webtoolkit.eu/");
link.setTarget(Wt::LinkTarget::NewWindow);

std::unique_ptr<Wt::WAnchor> anchor =
        std::make_unique<Wt::WAnchor>(link,
                        "Wt homepage (in a new window)");

SAMPLE_END(return std::move(anchor))
