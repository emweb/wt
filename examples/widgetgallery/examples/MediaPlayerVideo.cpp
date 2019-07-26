#include <Wt/WLink.h>
#include <Wt/WMediaPlayer.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(MediaPlayerVideo)

// Define media source locations
std::string mp4Video = "https://www.webtoolkit.eu/videos/sintel_trailer.mp4";
std::string ogvVideo = "https://www.webtoolkit.eu/videos/sintel_trailer.ogv";

// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WMediaPlayer *player = container->addNew<Wt::WMediaPlayer>(Wt::MediaType::Video);
player->addSource(Wt::MediaEncoding::M4V, Wt::WLink(mp4Video));
player->addSource(Wt::MediaEncoding::OGV, Wt::WLink(ogvVideo));
player->addSource(Wt::MediaEncoding::PosterImage, Wt::WLink(poster));
player->setTitle("<a href=\"https://durian.blender.org/\""
		 "target=\"_blank\">Sintel</a>, (c) copyright Blender Foundation");

Wt::WText *out = container->addNew<Wt::WText>();

player->playbackStarted().connect([=] {
    out->setText("<p>Video playing</p>");
});

player->playbackPaused().connect([=] {
    out->setText("<p>Video paused</p>");
});

player->ended().connect([=] {
    out->setText("<p>Video ended</p>");
});

player->volumeChanged().connect([=] {
    out->setText("<p>Volume changed</p>");
});

SAMPLE_END(return std::move(container))

