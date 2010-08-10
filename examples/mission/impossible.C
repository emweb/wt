/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "CountDownWidget.h"

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *appl = new WApplication(env);

  new WText("<h1>Your mission</h1>", appl->root());
  WText *secret 
    = new WText("Your mission, Jim, should you accept, is to create solid "
		"web applications.",
		appl->root());

  new WBreak(appl->root()); new WBreak(appl->root());

  new WText("This program will quit in ", appl->root());
  CountDownWidget *countdown = new CountDownWidget(10, 0, 1000, appl->root());
  new WText(" seconds.", appl->root());

  new WBreak(appl->root()); new WBreak(appl->root());

  WPushButton *cancelButton = new WPushButton("Cancel!", appl->root());
  WPushButton *quitButton = new WPushButton("Quit", appl->root());
  quitButton->clicked().connect(appl, &WApplication::quit);

  countdown->done().connect(appl, &WApplication::quit);
  cancelButton->clicked().connect(countdown, &CountDownWidget::cancel);
  cancelButton->clicked().connect(cancelButton, &WFormWidget::disable);
  cancelButton->clicked().connect(secret, &WWidget::hide);

  return appl;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
