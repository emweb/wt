/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WConfig.h>

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WProgressBar>
#include <Wt/Test/WTestEnvironment>

using namespace Wt;

namespace {

class BigWorkWidget : public Wt::WContainerWidget
{
public:
  BigWorkWidget(Wt::WContainerWidget *parent)
    : WContainerWidget(parent)
  {
    startButton_ = new Wt::WPushButton("Start", this);
    startButton_->setObjectName("startbutton");
    startButton_->clicked().connect(startButton_, &Wt::WPushButton::disable);
    startButton_->clicked().connect(this, &BigWorkWidget::startBigWork);
    startButton_->setMargin(2);

    progress_ = new Wt::WProgressBar(this);
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
}

BOOST_AUTO_TEST_CASE( test_serverpush_test )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  BigWorkWidget *bw = new BigWorkWidget(app.root());

  Wt::WPushButton *b
    = dynamic_cast<Wt::WPushButton *>(app.findWidget("startbutton"));

  Wt::WProgressBar *bar
    = dynamic_cast<Wt::WProgressBar *>(app.findWidget("progress"));

  b->clicked().emit(Wt::WMouseEvent());

  environment.endRequest();

  for (;;) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(50));
    std::cerr << "Progress: " << bar->value() << std::endl;
    if (b->isEnabled())
      break;
  }

  environment.startRequest();
}

#endif // WT_THREADED
