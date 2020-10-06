/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

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
  WText     *greeting_;

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

  root()->addWidget(std::make_unique<WText>("Your name, please? "));
  nameEdit_ = root()->addWidget(std::make_unique<WLineEdit>());
  nameEdit_->setFocus();

  auto button = root()->addWidget(std::make_unique<WPushButton>("Greet me"));
  button->setMargin(5, Side::Left);

  root()->addWidget(std::make_unique<WBreak>());

  greeting_ = root()->addWidget(std::make_unique<WText>());

  button->clicked().connect(this, &HelloApplication::greet);
  nameEdit_->enterPressed().connect
    (std::bind(&HelloApplication::greet, this));
}

void HelloApplication::destroy()
{
  root()->clear();
}

void HelloApplication::greet()
{
  /*
   * You can even use functions that block the event loop (and start a recursive
   * event loop).
   */
  WMessageBox::show("Welcome", "Hello there, " + nameEdit_->text(), StandardButton::Ok);
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<HelloApplication>(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

