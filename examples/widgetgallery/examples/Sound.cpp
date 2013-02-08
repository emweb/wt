#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WSound>
#include <Wt/WText>

SAMPLE_BEGIN(Sound)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WSound *sound = new Wt::WSound("sounds/beep.mp3", container);
sound->setLoops(3);

Wt::WPushButton *playButton = new Wt::WPushButton("Beep!", container);
playButton->setMargin(5);

Wt::WPushButton *stopButton = new Wt::WPushButton("Stop it!", container);
stopButton->setMargin(5);

Wt::WText *out = new Wt::WText(container);

playButton->clicked().connect(std::bind([=] () {
    sound->play();
    out->setText("<p>Beeping started!</p>");
}));

stopButton->clicked().connect(std::bind([=] () {
    sound->stop();
    out->setText("<p>Beeping stopped!</p>");
}));

SAMPLE_END(return container)


