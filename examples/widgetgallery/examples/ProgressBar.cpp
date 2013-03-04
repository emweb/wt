#include <Wt/WContainerWidget>
#include <Wt/WProgressBar>
#include <Wt/WPushButton>
#include <Wt/WTimer>

SAMPLE_BEGIN(ProgressBar)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->setStyleClass("inline-buttons");

Wt::WProgressBar *bar = new Wt::WProgressBar(container);
bar->setRange(0, 10);

Wt::WPushButton *startButton = new Wt::WPushButton("Start", container);
Wt::WPushButton *stopButton = new Wt::WPushButton("Stop", container);
Wt::WPushButton *resetButton = new Wt::WPushButton("Reset", container);

// Initially, only the start button is enabled.
stopButton->disable();
resetButton->disable();

// setup an interval timer which generates a timeout() signal every second.
Wt::WTimer *intervalTimer = new Wt::WTimer(container);
intervalTimer->setInterval(1000);

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

SAMPLE_END(return container)
