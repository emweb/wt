#include <Wt/WFlashObject>
#include <Wt/WContainerWidget>
#include <Wt/WFlashObject>
#include <Wt/WImage>
#include <Wt/WLink>
#include <Wt/WText>
#include <Wt/WVideo>

SAMPLE_BEGIN(VideoFallback)

// Define media source locations
std::string mp4Video = "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
std::string ogvVideo = "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";

// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WFlashObject *flash =
    new Wt::WFlashObject("http://www.webtoolkit.eu/videos/player_flv_maxi.swf");
flash->setFlashVariable("startimage", "pics/sintel_trailer.jpg");
flash->setFlashParameter("allowFullScreen", "true");
flash->setFlashVariable("flv", mp4Video);
flash->setFlashVariable("showvolume", "1");
flash->setFlashVariable("showfullscreen", "1");
flash->setAlternativeContent(new Wt::WImage(poster));
flash->resize(640, 360);

Wt::WVideo *video = new Wt::WVideo(container);
video->addSource(Wt::WLink(mp4Video));
video->addSource(Wt::WLink(ogvVideo));
video->setAlternativeContent(flash);
video->setPoster(poster);
video->resize(640, 360);

Wt::WText *out = new Wt::WText(container);

video->playbackStarted().connect(std::bind([=] () {
        out->setText("<p>Video playing</p>");
}));

video->playbackPaused().connect(std::bind([=] () {
        out->setText("<p>Video paused</p>");
}));

video->ended().connect(std::bind([=] () {
        out->setText("<p>Video ended</p>");
}));

video->volumeChanged().connect(std::bind([=] () {
        out->setText("<p>Volume changed</p>");
}));

SAMPLE_END(return container)
