// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef LOGIN_H_
#define LOGIN_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>

#include "MyCaptcha.h"

using namespace Wt;

class Login : public WContainerWidget
{
public:
  Login();

  Signal<WString>& loggedIn() { return loggedIn_; }

private:
  void captchaCompleted();
  void userNameEnterPressed();
  void loginClicked(const WMouseEvent& me);
  void login();

private:
  Signal<WString>   loggedIn_;
  WLineEdit        *userNameEdit_;
  MyCaptcha        *captcha_;
  WPushButton      *loginButton_;
};

#endif // LOGIN_H_
