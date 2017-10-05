#include <Wt/WAudio.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLink.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Audio)

// Define media source locations.
std::string mp3Audio =
            "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.mp3";
std::string oggAudio =
            "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.ogg";

auto container(Wt::cpp14::make_unique<Wt::WContainerWidget>());

Wt::WAudio *audio = container->addWidget(Wt::cpp14::make_unique<Wt::WAudio>());
audio->addSource(Wt::WLink(mp3Audio));
audio->addSource(Wt::WLink(oggAudio));
audio->setOptions(Wt::PlayerOption::Controls);
audio->setAlternativeContent
  (Wt::cpp14::make_unique<Wt::WText>("You don't have HTML5 audio support!"));

Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());

audio->playbackStarted().connect([=] {
        out->setText("<p>Audio playing</p>");
});

audio->playbackPaused().connect([=] {
        out->setText("<p>Audio paused</p>");
});

audio->ended().connect([=] {
        out->setText("<p>Audio ended</p>");
});

audio->volumeChanged().connect([=] {
        out->setText("<p>Volume changed</p>");
});

SAMPLE_END(return std::move(container))

