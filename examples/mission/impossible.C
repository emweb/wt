/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "CountDownWidget.h"

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> appl
      = cpp14::make_unique<WApplication>(env);

  appl->root()->addWidget(cpp14::make_unique<WText>("<h1>Your mission</h1>"));
  WText *secret 
    = appl->root()->addWidget(cpp14::make_unique<WText>("Your mission, Jim, should you accept, is to create solid "
                "web applications."));

  appl->root()->addWidget(cpp14::make_unique<WBreak>());
  appl->root()->addWidget(cpp14::make_unique<WBreak>());

  appl->root()->addWidget(cpp14::make_unique<WText>("This program will quit in "));
  CountDownWidget *countdown = appl->root()->addWidget(cpp14::make_unique<CountDownWidget>(10, 0, std::chrono::milliseconds{1000}));
  appl->root()->addWidget(cpp14::make_unique<WText>(" seconds."));

  appl->root()->addWidget(cpp14::make_unique<WBreak>());
  appl->root()->addWidget(cpp14::make_unique<WBreak>());

  WPushButton *cancelButton = appl->root()->addWidget(cpp14::make_unique<WPushButton>("Cancel!"));
  WPushButton *quitButton = appl->root()->addWidget(cpp14::make_unique<WPushButton>("Quit"));
  quitButton->clicked().connect(appl.get(), &WApplication::quit);

  countdown->done().connect([](){ WApplication::instance()->quit(); });
  cancelButton->clicked().connect(countdown, &CountDownWidget::cancel);
  cancelButton->clicked().connect(cancelButton, &WFormWidget::disable);
  cancelButton->clicked().connect(secret, &WWidget::hide);

  return appl;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
