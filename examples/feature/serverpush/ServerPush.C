/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WProgressBar>

#include <iostream>
#include <boost/thread.hpp>

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
class BigWorkWidget : public Wt::WContainerWidget
{
public:
  BigWorkWidget(Wt::WContainerWidget *parent)
    : WContainerWidget(parent)
  {
    startButton_ = new Wt::WPushButton("Start", this);
    startButton_->clicked().connect(startButton_, &Wt::WPushButton::disable);
    startButton_->clicked().connect(this, &BigWorkWidget::startBigWork);
    startButton_->setMargin(2);

    progress_ = new Wt::WProgressBar(this);
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

  boost::thread workThread_;

  void startBigWork() {
    Wt::WApplication *app = Wt::WApplication::instance();

    // Enable server push
    app->enableUpdates(true);

    workThread_ 
      = boost::thread(boost::bind(&BigWorkWidget::doBigWork, this, app));

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
      boost::this_thread::sleep(boost::posix_time::milliseconds(50));

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

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  Wt::WApplication *app = new Wt::WApplication(env);
  app->setCssTheme("polished");
  new BigWorkWidget(app->root());

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
