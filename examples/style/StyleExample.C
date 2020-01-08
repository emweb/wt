/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WLineEdit.h>
#include <Wt/WIntValidator.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>

#include "RoundedWidget.h"
#include "StyleExample.h"

char loremipsum[] = "Lorem ipsum dolor sit amet, consectetur adipisicing "
   "elit, sed do eiusmod tempor incididunt ut labore et "
   "dolore magna aliqua. Ut enim ad minim veniam, quis "
   "nostrud exercitation ullamco laboris nisi ut aliquip " 
   "ex ea commodo consequat. Duis aute irure dolor in "
   "reprehenderit in voluptate velit esse cillum dolore eu "
   "fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
   "non proident, sunt in culpa qui officia deserunt mollit "
   "anim id est laborum.";

StyleExample::StyleExample()
  : WContainerWidget()
{
  w_ = this->addWidget(cpp14::make_unique<RoundedWidget>());

  w_->contents()->addWidget(cpp14::make_unique<WText>(loremipsum));
  this->addWidget(cpp14::make_unique<WBreak>());

  this->addWidget(cpp14::make_unique<WText>("Color (rgb): "));
  r_ = createValidateLineEdit(w_->backgroundColor().red(), 0, 255);
  g_ = createValidateLineEdit(w_->backgroundColor().green(), 0, 255);
  b_ = createValidateLineEdit(w_->backgroundColor().blue(), 0, 255);

  this->addWidget(cpp14::make_unique<WBreak>());

  this->addWidget(cpp14::make_unique<WText>("Radius (px): "));
  radius_ = createValidateLineEdit(w_->cornerRadius(), 1, 500);

  this->addWidget(cpp14::make_unique<WBreak>());

  WPushButton *p = this->addWidget(cpp14::make_unique<WPushButton>("Update!"));
  p->clicked().connect(this, &StyleExample::updateStyle);

  this->addWidget(cpp14::make_unique<WBreak>());

  error_ = this->addWidget(cpp14::make_unique<WText>(""));
}

WLineEdit *StyleExample::createValidateLineEdit(int value, int min, int max)
{
  WLineEdit *le = this->addWidget(cpp14::make_unique<WLineEdit>(std::to_string(value)));
  le->setTextSize(3);
  le->setValidator(std::make_shared<WIntValidator>(min,max));

  return le;
}

void StyleExample::updateStyle()
{
  if ((r_->validate() != ValidationState::Valid)
      || (g_->validate() != ValidationState::Valid)
      || (b_->validate() != ValidationState::Valid))
    error_->setText("Color components must be numbers between 0 and 255.");
  else if (radius_->validate() != ValidationState::Valid)
    error_->setText("Radius must be between 1 and 500.");
  else {
    int r = std::atoi(r_->text().toUTF8().c_str());
    int g = std::atoi(g_->text().toUTF8().c_str());
    int b = std::atoi(b_->text().toUTF8().c_str());
    int radius = std::atoi(radius_->text().toUTF8().c_str());
    w_->setBackgroundColor(WColor(r, g, b));
    w_->setCornerRadius(radius);
    error_->setText("");
  }
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> app
      = cpp14::make_unique<WApplication>(env);
  app->setTitle("Style example");

  app->root()->addWidget(cpp14::make_unique<StyleExample>());
  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
