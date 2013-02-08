#include <Wt/WLink>
#include <Wt/WMediaPlayer>
#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(MediaPlayerVideo)
// Define media source locations
std::string mp4Video = "http://www.webtoolkit.eu/videos/sintel_trailer.mp4";
std::string ogvVideo = "http://www.webtoolkit.eu/videos/sintel_trailer.ogv";

// Define poster image location
std::string poster = "pics/sintel_trailer.jpg";

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WMediaPlayer *player =
    new Wt::WMediaPlayer(Wt::WMediaPlayer::Video, container);
player->addSource(Wt::WMediaPlayer::M4V, Wt::WLink(mp4Video));
player->addSource(Wt::WMediaPlayer::OGV, Wt::WLink(ogvVideo));
player->addSource(Wt::WMediaPlayer::PosterImage, Wt::WLink(poster));
player->setTitle("<a href=\"http://durian.blender.org/\""
		 "target=\"_blank\">Sintel</a>, (c) copyright Blender Foundation");

Wt::WText *out = new Wt::WText(container);

player->playbackStarted().connect(std::bind([=] () {
    out->setText("<p>Video playing</p>");
}));

player->playbackPaused().connect(std::bind([=] () {
    out->setText("<p>Video paused</p>");
}));

player->ended().connect(std::bind([=] () {
    out->setText("<p>Video ended</p>");
}));

player->volumeChanged().connect(std::bind([=] () {
    out->setText("<p>Volume changed</p>");
}));

SAMPLE_END(return container)

