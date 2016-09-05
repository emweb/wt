/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WServer>
#include <Wt/WString>
#include <Wt/WText>

using namespace Wt;

class ScrollApplication : public WApplication {
public:
  ScrollApplication(const WEnvironment &env);

private:
  void addSentinel();
  void removeSentinel();
  void visibilityChanged(bool visible);

  WContainerWidget *sentinel_;
  int i_;
};

ScrollApplication::ScrollApplication(const WEnvironment &env)
  : WApplication(env),
    i_(0)
{
  setTitle("Wt scroll visibility example");

  root()->addWidget(new WText("<h1>Wt scroll visibility example</h1>"));
  root()->addWidget(new WText("This example illustrates the use of <tt>scrollVisibilityChanged()</tt> "
			      "to trigger the loading of more content. Scroll down to the bottom "
			      "and more text will appear."));

  for (int i = 0; i < 100; ++i) {
    root()->addWidget(new WText(WString("<br/>WText widget {1}").arg(i_)));
    ++i_;
  }

  addSentinel();
}

// Add a sentinel WContainerWidget that triggers the loading of more text widgets
// when it is 200 pixels below the window.
void ScrollApplication::addSentinel()
{
  sentinel_ = new WContainerWidget();
  sentinel_->setScrollVisibilityEnabled(true);
  sentinel_->setScrollVisibilityMargin(200);
  sentinel_->scrollVisibilityChanged().connect(this, &ScrollApplication::visibilityChanged);
  sentinel_->setHeight(100);
  root()->addWidget(sentinel_);
}

// Removes the sentinel WContainerWidget
void ScrollApplication::removeSentinel()
{
  root()->removeWidget(sentinel_);
  delete sentinel_;
}

// Triggered when the sentinel comes into view. Adds more WText widgets and
// installs a new sentinel.
void ScrollApplication::visibilityChanged(bool visible)
{
  if (visible) {
    removeSentinel();

    for (int i = 0; i < 100; ++i) {
      root()->addWidget(new WText(WString("<br/>WText widget {1}").arg(i_)));
      ++i_;
    }

    if (i_ < 1000)
      addSentinel();
    else {
      root()->addWidget(new WText("<br/><b>Limiting to 1000 widgets</b>"));
    }
  }
}

WApplication *createApplication(const WEnvironment &env)
{
  return new ScrollApplication(env);
}

int main(int argc, char *argv[]) {
  return WRun(argc, argv, &createApplication);
}
