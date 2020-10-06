/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Login.h"

#include <Wt/WLabel.h>

Login::Login()
  : WContainerWidget()
{
  setStyleClass("login");
		 
  WLabel* userNameL =
      this->addWidget(std::make_unique<WLabel>(tr("login.userName")));
  userNameEdit_ = this->addWidget(std::make_unique<WLineEdit>());
  userNameEdit_->setFocus();
  userNameEdit_->setValidator(std::make_shared<WValidator>(true));
  userNameL->setBuddy(userNameEdit_);

  userNameEdit_->enterPressed().connect(this, &Login::userNameEnterPressed);
  
  loginButton_ =
      this->addWidget(std::make_unique<WPushButton>(tr("login.loginButton")));
  loginButton_->hide();
  loginButton_->clicked().connect(this, &Login::loginClicked);

  captcha_ =
      this->addWidget(std::make_unique<MyCaptcha>(150, 70));
  captcha_->completed().connect(this, &Login::captchaCompleted);
}

void Login::captchaCompleted()
{
  if (userNameEdit_->validate() != ValidationState::Valid) {
    captcha_->hide();
    loginButton_->show();
    userNameEdit_->setFocus();
  } else {
    login();
  }
}

void Login::userNameEnterPressed()
{
  if (userNameEdit_->validate() == ValidationState::Valid
      && !loginButton_->isHidden())
    login();
}

void Login::loginClicked(const WMouseEvent& me)
{
  if (userNameEdit_->validate() == ValidationState::Valid)
    login();
}

void Login::login()
{
  loggedIn_.emit(userNameEdit_->text());
}
