/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include <Wt/WLineEdit>
#include <Wt/WIntValidator>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WApplication>
#include <Wt/WBreak>

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

StyleExample::StyleExample(WContainerWidget *parent)
  : WContainerWidget(parent)
{
  w_ = new RoundedWidget(RoundedWidget::All, this);

  new WText(loremipsum, w_->contents());
  new WBreak(this);

  new WText("Color (rgb): ", this);
  r_ = createValidateLineEdit(w_->backgroundColor().red(), 0, 255);
  g_ = createValidateLineEdit(w_->backgroundColor().green(), 0, 255);
  b_ = createValidateLineEdit(w_->backgroundColor().blue(), 0, 255);

  new WBreak(this);

  new WText("Radius (px): ", this);
  radius_ = createValidateLineEdit(w_->cornerRadius(), 1, 500);

  new WBreak(this);

  WPushButton *p = new WPushButton("Update!", this);
  p->clicked().connect(this, &StyleExample::updateStyle);

  new WBreak(this);

  error_ = new WText("", this);
}

WLineEdit *StyleExample::createValidateLineEdit(int value, int min, int max)
{
  WLineEdit *le = new WLineEdit(boost::lexical_cast<std::wstring>(value), this);
  le->setTextSize(3);
  le->setValidator(new WIntValidator(min, max));

  return le;
}

void StyleExample::updateStyle()
{
  if ((r_->validate() != WValidator::Valid)
      || (g_->validate() != WValidator::Valid)
      || (b_->validate() != WValidator::Valid))
    error_->setText("Color components must be numbers between 0 and 255.");
  else if (radius_->validate() != WValidator::Valid)
    error_->setText("Radius must be between 1 and 500.");
  else {
    w_->setBackgroundColor(WColor(boost::lexical_cast<int>(r_->text()),
				  boost::lexical_cast<int>(g_->text()),
				  boost::lexical_cast<int>(b_->text())));
    w_->setCornerRadius(boost::lexical_cast<int>(radius_->text()));
    error_->setText("");
  }
}

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  app->setTitle("Style example");

  app->root()->addWidget(new StyleExample());
  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
