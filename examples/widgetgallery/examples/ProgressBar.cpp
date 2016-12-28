#include <Wt/WContainerWidget.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WTimer.h>

SAMPLE_BEGIN(ProgressBar)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
container->setStyleClass("inline-buttons");

Wt::WProgressBar *bar =
    container->addWidget(Wt::cpp14::make_unique<Wt::WProgressBar>());
bar->setRange(0, 10);

Wt::WPushButton *startButton =
    container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Start"));
Wt::WPushButton *stopButton =
    container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Stop"));
Wt::WPushButton *resetButton =
    container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Reset"));

// Initially, only the start button is enabled.
stopButton->disable();
resetButton->disable();

// setup an interval timer which generates a timeout() signal every second.
auto intervalTimerPtr = Wt::cpp14::make_unique<Wt::WTimer>();
auto intervalTimer = intervalTimerPtr.get();
container->addChild(std::move(intervalTimerPtr));
intervalTimer->setInterval(std::chrono::milliseconds{1000});

startButton->clicked().connect(std::bind([=] () {
    if (bar->value() < 10) {
        intervalTimer->start();
	startButton->setText("Resume");
    }

    startButton->disable();
    stopButton->enable();
    resetButton->disable();
}));

stopButton->clicked().connect(std::bind([=] () {
    intervalTimer->stop();

    startButton->enable();
    stopButton->disable();
    resetButton->enable();
}));

resetButton->clicked().connect(std::bind([=] () {
    bar->setValue(0.0);
    startButton->setText("Start");

    startButton->enable();
    stopButton->disable();
    resetButton->disable();
}));

intervalTimer->timeout().connect(std::bind([=] () {
    bar->setValue(bar->value() + 1);
    if (bar->value() == 10) {
        stopButton->clicked().emit(Wt::WMouseEvent());
	startButton->disable();
    }
}));

SAMPLE_END(return std::move(container))
