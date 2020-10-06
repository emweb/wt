#include <Wt/WFlashObject.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WFlashObject.h>
#include <Wt/WImage.h>
#include <Wt/WLink.h>
#include <Wt/WText.h>
#include <Wt/WVideo.h>

SAMPLE_BEGIN(VideoFallback)

// Define media source locations
std::string mp4Video = "https://www.webtoolkit.eu/videos/sintel_trailer.mp4";
std::string ogvVideo = "https://www.webtoolkit.eu/videos/sintel_trailer.ogv";

// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

auto container = std::make_unique<Wt::WContainerWidget>();

auto flash =
    std::make_unique<Wt::WFlashObject>("https://www.webtoolkit.eu/videos/player_flv_maxi.swf");
flash->setFlashVariable("startimage", "pics/sintel_trailer.jpg");
flash->setFlashParameter("allowFullScreen", "true");
flash->setFlashVariable("flv", mp4Video);
flash->setFlashVariable("showvolume", "1");
flash->setFlashVariable("showfullscreen", "1");
flash->setAlternativeContent(std::make_unique<Wt::WImage>(Wt::WLink(poster)));
flash->resize(640, 360);

Wt::WVideo *video = container->addNew<Wt::WVideo>();
video->addSource(Wt::WLink(mp4Video));
video->addSource(Wt::WLink(ogvVideo));
video->setAlternativeContent(std::move(flash));
video->setPoster(poster);
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
