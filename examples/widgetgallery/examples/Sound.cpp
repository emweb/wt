#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WSound.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Sound)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

#ifndef WT_TARGET_JAVA
auto sound = container->addChild(
	Wt::cpp14::make_unique<Wt::WSound>("sounds/beep.mp3"));
#else // WT_TARGET_JAVA
auto sound = new Wt::WSound("sounds/beep.mp3");
#endif // WT_TARGET_JAVA
sound->setLoops(3);

auto playButton = container->addNew<Wt::WPushButton>("Beep!");
playButton->setMargin(5);

auto stopButton = container->addNew<Wt::WPushButton>("Stop it!");
stopButton->setMargin(5);

Wt::WText *out = container->addNew<Wt::WText>();

playButton->clicked().connect([=] {
    sound->play();
    out->setText("<p>Beeping started!</p>");
});

stopButton->clicked().connect([=] {
    sound->stop();
    out->setText("<p>Beeping stopped!</p>");
});

SAMPLE_END(return std::move(container))


