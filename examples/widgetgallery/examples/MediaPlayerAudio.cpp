#include <Wt/WContainerWidget>
#include <Wt/WLink>
#include <Wt/WMediaPlayer>
#include <Wt/WText>

SAMPLE_BEGIN(MediaPlayerAudio)
// Define media source locations.
std::string mp3Audio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.mp3";
std::string oggAudio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.ogg";

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WMediaPlayer *player =
    new Wt::WMediaPlayer(Wt::WMediaPlayer::Audio, container);
player->addSource(Wt::WMediaPlayer::MP3, Wt::WLink(mp3Audio));
player->addSource(Wt::WMediaPlayer::OGA, Wt::WLink(oggAudio));
player->setTitle("La Sera - Never Come Around");

Wt::WText *out = new Wt::WText(container);

player->playbackStarted().connect(std::bind([=] () {
    out->setText("<p>Song playing</p>");
}));

player->playbackPaused().connect(std::bind([=] () {
    out->setText("<p>Song paused</p>");
}));

player->ended().connect(std::bind([=] () {
    out->setText("<p>Song ended</p>");
}));

player->volumeChanged().connect(std::bind([=] () {
    out->setText("<p>Volume changed</p>");
}));

SAMPLE_END(return container)

