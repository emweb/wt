#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WLink.h>
#include <Wt/WFlashObject.h>

SAMPLE_BEGIN(Flash)
// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

auto container = std::make_unique<Wt::WContainerWidget>();

auto flash = container->addNew<Wt::WFlashObject>("https://www.youtube.com/v/HOfdboHvshg");
flash->setFlashParameter("allowFullScreen", "true");
flash->setAlternativeContent(std::make_unique<Wt::WImage>(Wt::WLink(poster)));
flash->resize(640, 360);

SAMPLE_END(return std::move(container))

