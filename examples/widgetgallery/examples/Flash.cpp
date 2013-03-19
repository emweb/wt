#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WFlashObject>

SAMPLE_BEGIN(Flash)
// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WFlashObject *flash =
    new Wt::WFlashObject("http://www.youtube.com/v/HOfdboHvshg", container);
flash->setFlashParameter("allowFullScreen", "true");
flash->setAlternativeContent(new Wt::WImage(poster));
flash->resize(640, 360);

SAMPLE_END(return container)

