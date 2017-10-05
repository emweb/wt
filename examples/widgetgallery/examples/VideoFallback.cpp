#include <Wt/WFlashObject.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WFlashObject.h>
#include <Wt/WImage.h>
#include <Wt/WLink.h>
#include <Wt/WText.h>
#include <Wt/WVideo.h>

SAMPLE_BEGIN(VideoFallback)

// Define media source locations
std::string mp4Video = "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
std::string ogvVideo = "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";

// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

auto flash =
    Wt::cpp14::make_unique<Wt::WFlashObject>("http://www.webtoolkit.eu/videos/player_flv_maxi.swf");
flash->setFlashVariable("startimage", "pics/sintel_trailer.jpg");
flash->setFlashParameter("allowFullScreen", "true");
flash->setFlashVariable("flv", mp4Video);
flash->setFlashVariable("showvolume", "1");
flash->setFlashVariable("showfullscreen", "1");
flash->setAlternativeContent(Wt::cpp14::make_unique<Wt::WImage>(poster));
flash->resize(640, 360);

Wt::WVideo *video = container->addWidget(Wt::cpp14::make_unique<Wt::WVideo>());
video->addSource(Wt::WLink(mp4Video));
video->addSource(Wt::WLink(ogvVideo));
video->setAlternativeContent(std::move(flash));
video->setPoster(poster);
video->resize(640, 360);

Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());

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
