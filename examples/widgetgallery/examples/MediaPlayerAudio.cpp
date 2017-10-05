#include <Wt/WContainerWidget.h>
#include <Wt/WLink.h>
#include <Wt/WMediaPlayer.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(MediaPlayerAudio)

// Define media source locations.
std::string mp3Audio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.mp3";
std::string oggAudio =
    "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.ogg";

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WMediaPlayer *player =
    container->addWidget(Wt::cpp14::make_unique<Wt::WMediaPlayer>(Wt::MediaType::Audio));
player->addSource(Wt::MediaEncoding::MP3, Wt::WLink(mp3Audio));
player->addSource(Wt::MediaEncoding::OGA, Wt::WLink(oggAudio));
player->setTitle("La Sera - Never Come Around");

Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());

player->playbackStarted().connect([=] {
    out->setText("<p>Song playing</p>");
});

player->playbackPaused().connect([=] {
    out->setText("<p>Song paused</p>");
});

player->ended().connect([=] {
    out->setText("<p>Song ended</p>");
});

player->volumeChanged().connect([=] {
    out->setText("<p>Volume changed</p>");
});

SAMPLE_END(return std::move(container))

