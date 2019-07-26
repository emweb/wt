#include <Wt/WContainerWidget.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WTimer.h>

SAMPLE_BEGIN(ProgressBar)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
container->setStyleClass("inline-buttons");

Wt::WProgressBar *bar = container->addNew<Wt::WProgressBar>();
bar->setRange(0, 10);

Wt::WPushButton *startButton = container->addNew<Wt::WPushButton>("Start");
Wt::WPushButton *stopButton = container->addNew<Wt::WPushButton>("Stop");
Wt::WPushButton *resetButton = container->addNew<Wt::WPushButton>("Reset");

// Initially, only the start button is enabled.
stopButton->disable();
resetButton->disable();

// setup an interval timer which generates a timeout() signal every second.
auto intervalTimer = container->addChild(Wt::cpp14::make_unique<Wt::WTimer>());
intervalTimer->setInterval(std::chrono::milliseconds(1000));

startButton->clicked().connect([=] {
    if (bar->value() < 10) {
        intervalTimer->start();
	startButton->setText("Resume");
    }

    startButton->disable();
    stopButton->enable();
    resetButton->disable();
});

stopButton->clicked().connect([=] {
    intervalTimer->stop();

    startButton->enable();
    stopButton->disable();
    resetButton->enable();
});

resetButton->clicked().connect([=] {
    bar->setValue(0.0);
    startButton->setText("Start");

    startButton->enable();
    stopButton->disable();
    resetButton->disable();
});

intervalTimer->timeout().connect([=] {
    bar->setValue(bar->value() + 1);
    if (bar->value() == 10) {
        stopButton->clicked().emit(Wt::WMouseEvent());
	startButton->disable();
    }
});

SAMPLE_END(return std::move(container))
