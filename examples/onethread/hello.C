/*
 * Copyright (C) 2014 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "SingleThreadedApplication.h"

using namespace Wt;

class HelloApplication : public SingleThreadedApplication
{
public:
  HelloApplication(const WEnvironment& env);

protected:
  virtual void create();
  virtual void destroy();

private:
  WLineEdit *nameEdit_;
  WText *greeting_;

  void greet();
};

HelloApplication::HelloApplication(const WEnvironment& env)
  : SingleThreadedApplication(env)
{
  /* Intentionally left blank! Actual construction happens in create() */
}

void HelloApplication::create()
{
  setTitle("Hello world");

  root()->addWidget(new WText("Your name, please ? "));
  nameEdit_ = new WLineEdit(root());
  nameEdit_->setFocus();

  WPushButton *button = new WPushButton("Greet me.", root());
  button->setMargin(5, Left);

  root()->addWidget(new WBreak());

  greeting_ = new WText(root());

  button->clicked().connect(this, &HelloApplication::greet);
  nameEdit_->enterPressed().connect
    (boost::bind(&HelloApplication::greet, this));
}

void HelloApplication::destroy()
{
  root()->clear();
}

void HelloApplication::greet()
{
  /*
   * You can even functions that block the event loop (and start a recursive
   * event loop).
   */
  WMessageBox::show("Welcome", "Hello there, " + nameEdit_->text(), Ok);
}

WApplication *createApplication(const WEnvironment& env)
{
  return new HelloApplication(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

