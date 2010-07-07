#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WText>

#include <iostream>
#include <boost/thread.hpp>

using namespace Wt;

/*
 * This is a minimal server push example, which is used to update the GUI
 * while a big work is computing in another thread.
 */

class BigWorkWidget : public WContainerWidget
{
public:
  BigWorkWidget(WContainerWidget *parent)
    : WContainerWidget(parent)
  {
    startButton_ = new WPushButton("Start", this);
    startButton_->clicked().connect(startButton_, &WPushButton::disable);
    startButton_->clicked().connect(this, &BigWorkWidget::startBigWork);

    resultText_ = new WText(this);
    resultText_->setInline(false);
  }

private:
  WPushButton *startButton_;
  WText *resultText_;

  boost::thread workThread_;

  void startBigWork() {
    WApplication *app = WApplication::instance();

    // Enable server push
    app->enableUpdates(true);

    workThread_ 
      = boost::thread(boost::bind(&BigWorkWidget::doBigWork, this, app));

    resultText_->setText("");
    startButton_->setText("Busy...");
  }

  /*
   * This function runs from another thread.
   *
   * From within this thread, we cannot use WApplication::instance(), since
   * that use thread-local storage. We can only access WApplication::instance()
   * after we have grabbed its update-lock.
   */
  void doBigWork(WApplication *app)
  {
    for (unsigned i = 0; i < 20; ++i) {
      // Do 50 ms of hard work.
      boost::this_thread::sleep(boost::posix_time::milliseconds(50));

      // Get the application update lock to update the user-interface
      // with a progress indication.
      WApplication::UpdateLock uiLock = app->getUpdateLock();

      resultText_->setText(resultText_->text() + ".");

      app->triggerUpdate();
    }


    WApplication::UpdateLock uiLock = app->getUpdateLock();

    resultText_->setText("That was hefty!");
    startButton_->enable();
    startButton_->setText("Again!");

    app->triggerUpdate();

    // Disable server push
    app->enableUpdates(false);
  }
};

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  new BigWorkWidget(app->root());

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
