/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Login.h"

#include <Wt/WLabel>

using namespace Wt;

Login::Login(WContainerWidget* parent)
  : WContainerWidget(parent)
{
  setStyleClass("login");
		 
  WLabel* userNameL = new WLabel(tr("login.userName"), this);
  userNameEdit_ = new WLineEdit(this);
  userNameEdit_->setFocus();
  userNameEdit_->setValidator(new WValidator(true));
  userNameL->setBuddy(userNameEdit_);

  userNameEdit_->enterPressed().
    connect(SLOT(this, Login::userNameEnterPressed));
  
  loginButton_ = new WPushButton(tr("login.loginButton"), this);
  loginButton_->hide();
  loginButton_->clicked().connect(SLOT(this, Login::loginClicked));

  captcha_ = new MyCaptcha(this, 150, 70);
  captcha_->completed.connect(SLOT(this, Login::captchaCompleted));
}

void Login::captchaCompleted()
{
  if (userNameEdit_->validate() != WValidator::Valid) {
    captcha_->hide();
    loginButton_->show();
    userNameEdit_->setFocus();
  } else {
    login();
  }
}

void Login::userNameEnterPressed()
{
  if (userNameEdit_->validate() == WValidator::Valid 
      && !loginButton_->isHidden())
    login();
}

void Login::loginClicked(const WMouseEvent& me)
{
  if (userNameEdit_->validate() == WValidator::Valid)
    login();
}

void Login::login()
{
  //TODO trim
  loggedIn.emit(userNameEdit_->text().toUTF8());
}
