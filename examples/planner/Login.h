/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LOGIN_H_
#define LOGIN_H_

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>

#include <string>

#include "MyCaptcha.h"

class Login : public Wt::WContainerWidget {
 public:
  Login(Wt::WContainerWidget* parent);

 private:
  void captchaCompleted();
  void userNameEnterPressed();
  void loginClicked(const Wt::WMouseEvent& me);
  void login();

 public:
  Wt::Signal<std::string> loggedIn;

 private:
  Wt::WLineEdit *userNameEdit_;
  MyCaptcha *captcha_;
  Wt::WPushButton *loginButton_;
};

#endif //LOGIN_H_
