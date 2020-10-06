/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WConfig.h>

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>
#include <thread>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WProgressBar.h>
#include <Wt/Test/WTestEnvironment.h>

using namespace Wt;

namespace {

class BigWorkWidget : public Wt::WContainerWidget
{
public:
  BigWorkWidget()
    : WContainerWidget()
  {
    startButton_ = addWidget(std::make_unique<Wt::WPushButton>("Start"));
    startButton_->setObjectName("startbutton");
    startButton_->clicked().connect(startButton_, &Wt::WPushButton::disable);
    startButton_->clicked().connect(this, &BigWorkWidget::startBigWork);
    startButton_->setMargin(2);

    progress_ = addWidget(std::make_unique<Wt::WProgressBar>());
    progress_->setObjectName("progress");
    progress_->setInline(false);
    progress_->setMinimum(0);
    progress_->setMaximum(20);
    progress_->setMargin(2);
  }

  virtual ~BigWorkWidget() {
    workThread_.join();
  }

private:
  Wt::WPushButton *startButton_;
  Wt::WProgressBar *progress_;

  std::thread workThread_;

  void startBigWork() {
    Wt::WApplication *app = Wt::WApplication::instance();

    // Enable server push
    app->enableUpdates(true);

    workThread_ = std::thread(std::bind(&BigWorkWidget::doBigWork, this, app));

    progress_->setValue(0);
    startButton_->setText("Busy...");
  }

  /*
   * This function runs from another thread.
   *
   * From within this thread, we cannot use WApplication::instance(),
   * since that use thread-local storage. We can only access
   * WApplication::instance() after we have grabbed its update-lock.
   */
  void doBigWork(Wt::WApplication *app)
  {
    for (unsigned i = 0; i < 20; ++i) {
      // Do 50 ms of hard work.
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      // Get the application update lock to update the user-interface
      // with a progress indication.
      Wt::WApplication::UpdateLock uiLock(app);
      if (uiLock) {
	progress_->setValue(i + 1);
	app->triggerUpdate();
      }
    }

    Wt::WApplication::UpdateLock uiLock(app);

    if (uiLock) {
      startButton_->enable();
      startButton_->setText("Again!");

      app->triggerUpdate();

      // Disable server push
      app->enableUpdates(false);
    }
  }
};
}

BOOST_AUTO_TEST_CASE( test_serverpush_test )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  app.root()->addWidget(std::make_unique<BigWorkWidget>());

  Wt::WPushButton *b
    = dynamic_cast<Wt::WPushButton *>(app.findWidget("startbutton"));

  Wt::WProgressBar *bar
    = dynamic_cast<Wt::WProgressBar *>(app.findWidget("progress"));

  b->clicked().emit(Wt::WMouseEvent());

  environment.endRequest();

  for (;;) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cerr << "Progress: " << bar->value() << std::endl;
    if (b->isEnabled())
      break;
  }

  environment.startRequest();
}

#endif // WT_THREADED
