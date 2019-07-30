#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WLink.h>
#include <Wt/WText.h>
#include <Wt/WVideo.h>

SAMPLE_BEGIN(Video)

// Define media source locations
std::string mp4Video = "https://www.webtoolkit.eu/videos/sintel_trailer.mp4";
std::string ogvVideo = "https://www.webtoolkit.eu/videos/sintel_trailer.ogv";

// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

auto video = container->addNew<Wt::WVideo>();
video->addSource(Wt::WLink(mp4Video));
video->addSource(Wt::WLink(ogvVideo));
video->setPoster(poster);
video->setAlternativeContent(Wt::cpp14::make_unique<Wt::WImage>(Wt::WLink(poster)));
video->resize(640, 360);

Wt::WText *out = container->addNew<Wt::WText>();

video->playbackStarted().connect([=] {
        out->setText("<p>Video playing</p>");
});

video->playbackPaused().connect([=] {
        out->setText("<p>Video paused</p>");
});

video->ended().connect([=] {
        out->setText("<p>Video ended</p>");
});

video->volumeChanged().connect([=] {
        out->setText("<p>Volume changed</p>");
});

SAMPLE_END(return std::move(container))
