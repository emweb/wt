#include <Wt/WAudio>
#include <Wt/WContainerWidget>
#include <Wt/WLink>
#include <Wt/WText>

SAMPLE_BEGIN(Audio)
// Define media source locations.
std::string mp3Audio =
            "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.mp3";
std::string oggAudio =
            "http://www.webtoolkit.eu/audio/LaSera-NeverComeAround.ogg";

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WAudio *audio = new Wt::WAudio(container);
audio->addSource(Wt::WLink(mp3Audio));
audio->addSource(Wt::WLink(oggAudio));
audio->setOptions(Wt::WAudio::Controls);
audio->setAlternativeContent
  (new Wt::WText("You don't have HTML5 audio support!"));

Wt::WText *out = new Wt::WText(container);

audio->playbackStarted().connect(std::bind([=] () {
        out->setText("<p>Audio playing</p>");
}));

audio->playbackPaused().connect(std::bind([=] () {
        out->setText("<p>Audio paused</p>");
}));

audio->ended().connect(std::bind([=] () {
        out->setText("<p>Audio ended</p>");
}));

audio->volumeChanged().connect(std::bind([=] () {
        out->setText("<p>Volume changed</p>");
}));

SAMPLE_END(return container)

