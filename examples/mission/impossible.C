/*
 * Copyright (C) 2006 Koen Deforche
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

  new WText(L"<h1>Your mission</h1>", appl->root());
  WText *secret 
    = new WText(L"Your mission, Jim, should you accept, is to create solid "
		L"web applications.",
		appl->root());

  new WBreak(appl->root()); new WBreak(appl->root());

  new WText(L"This program will quit in ", appl->root());
  CountDownWidget *countdown = new CountDownWidget(10, 0, 1000, appl->root());
  new WText(L" seconds.", appl->root());

  new WBreak(appl->root()); new WBreak(appl->root());

  WPushButton *cancelButton = new WPushButton(L"Cancel!", appl->root());
  WPushButton *quitButton = new WPushButton(L"Quit", appl->root());
  quitButton->clicked.connect(SLOT(appl, WApplication::quit));

  countdown->done.connect(SLOT(appl, WApplication::quit));
  cancelButton->clicked.connect(SLOT(countdown, CountDownWidget::cancel));
  cancelButton->clicked.connect(SLOT(cancelButton, WFormWidget::disable));
  cancelButton->clicked.connect(SLOT(secret, WWidget::hide));

  return appl;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
