/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WProgressBar.h>

#include <iostream>
#include <thread>
#include <chrono>

using namespace Wt;

/*
 * This is a minimal server push example, which is used to update the GUI
 * while a big work is computing in another thread.
 *
 * This example grabs the userinterface UpdateLock to directly modify
 * an application's user-interface from a worker thread. This works
 * fine for a thread that was created and is owned by a single
 * application, doing work involving only that application.
 *
 * In more complex scenarios, it may be easier to use WServer::post()
 * to post an event to a session. This approach is illustrated in the
 * simplechat example.
 */
class BigWorkWidget : public WContainerWidget
{
public:
  BigWorkWidget()
    : WContainerWidget()
  {
    startButton_ = this->addWidget(cpp14::make_unique<WPushButton>("Start"));
    startButton_->clicked().connect(startButton_, &WPushButton::disable);
    startButton_->clicked().connect(this, &BigWorkWidget::startBigWork);
    startButton_->setMargin(2);

    progress_ = this->addWidget(cpp14::make_unique<WProgressBar>());
    progress_->setInline(false);
    progress_->setMinimum(0);
    progress_->setMaximum(20);
    progress_->setMargin(2);
  }

  virtual ~BigWorkWidget() {
    if (workThread_.get_id() != std::this_thread::get_id() &&
	workThread_.joinable())
      workThread_.join();
  }

private:
  WPushButton *startButton_;
  WProgressBar *progress_;

  std::thread workThread_;

  void startBigWork() {
    WApplication *app = WApplication::instance();

    // Enable server push
    app->enableUpdates(true);

    if (workThread_.joinable())
      workThread_.join();
    workThread_
      = std::thread(std::bind(&BigWorkWidget::doBigWork, this, app));

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
  void doBigWork(WApplication *app)
  {
    for (unsigned i = 0; i < 20; ++i) {
      // Do 50 ms of hard work.
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      // Get the application update lock to update the user-interface
      // with a progress indication.
      WApplication::UpdateLock uiLock(app);
      if (uiLock) {
        progress_->setValue(i + 1);
        app->triggerUpdate();
      } else
        return;
    }

    WApplication::UpdateLock uiLock(app);

    if (uiLock) {
      startButton_->enable();
      startButton_->setText("Again!");

      app->triggerUpdate();

      // Disable server push
      app->enableUpdates(false);
    } else
      return;
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> app = cpp14::make_unique<WApplication>(env);
  app->setCssTheme("polished");
  app->root()->addWidget(cpp14::make_unique<BigWorkWidget>());

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
