// This may look like C code, but it's really -*- C++ -*-
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

#include "MyCaptcha.h"

class Login : public Wt::WContainerWidget
{
public:
  Login(Wt::WContainerWidget* parent);

  Wt::Signal<Wt::WString>& loggedIn() { return loggedIn_; }

private:
  void captchaCompleted();
  void userNameEnterPressed();
  void loginClicked(const Wt::WMouseEvent& me);
  void login();

private:
  Wt::Signal<Wt::WString> loggedIn_;
  Wt::WLineEdit *userNameEdit_;
  MyCaptcha *captcha_;
  Wt::WPushButton *loginButton_;
};

#endif // LOGIN_H_
