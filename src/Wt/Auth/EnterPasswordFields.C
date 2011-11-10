/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/EnterPasswordFields"

#include "Wt/WApplication"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WStringStream"
#include "Wt/WText"

#ifndef WT_DEBUG_JS
#include "js/AuthEnterPasswordFields.min.js"
#endif

namespace Wt {
  namespace Auth {

EnterPasswordFields::EnterPasswordFields(const AbstractPasswordService& auth,
					 WLineEdit *password,
					 WText *passwordInfo,
					 WPushButton *okButton,
					 WObject *parent)
  : WObject(parent),
    auth_(auth),
    password_(password),
    okButton_(okButton),
    passwordInfo_(passwordInfo)
{
  init();
}

void EnterPasswordFields::init()
{
  password_->setEchoMode(WLineEdit::Password);
  if (passwordInfo_)
    passwordInfo_->setText(WString::tr("Wt.Auth.password-info"));

  if (auth_.attemptThrottlingEnabled())
    enableThrottlingJS();
}

void EnterPasswordFields::enableThrottlingJS()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/AuthEnterPasswordFields.js", "AuthThrottle", wtjs1);

  okButton_
    ->doJavaScript("new " WT_CLASS ".AuthThrottle(" WT_CLASS ","
		   + okButton_->jsRef() + ","
		   + WString::tr("Wt.Auth.throttle-retry").jsStringLiteral()
		   + ");");
}

void EnterPasswordFields::loginThrottle(int delay)
{
  WStringStream s;
  s << "jQuery.data(" << okButton_->jsRef() << ", 'throttle').reset("
    << delay << ");";

  okButton_->doJavaScript(s.str());
}

bool EnterPasswordFields::validate(const User& user)
{
  bool result = false;
  int throttlingDelay = 0;

  if (user.isValid()) {
    PasswordResult r = auth_.verifyPassword(user, password_->text());

    switch (r) {
    case PasswordInvalid:
      if (passwordInfo_) {
	passwordInfo_->setText(WString::tr("Wt.Auth.password-invalid"));
	passwordInfo_->addStyleClass("Wt-error");
      }
      password_->removeStyleClass("Wt-valid", true);
      password_->addStyleClass("Wt-invalid", true);
      if (auth_.attemptThrottlingEnabled())
	throttlingDelay = auth_.delayForNextAttempt(user);
      break;
    case LoginThrottling:
      password_->removeStyleClass("Wt-invalid", true);
      throttlingDelay = auth_.delayForNextAttempt(user);
      Wt::log("auth") << "Throttling: " << throttlingDelay
		      << " seconds for " << user.identity("username");
      break;
    case PasswordValid:
      password_->removeStyleClass("Wt-invalid", true);
      password_->addStyleClass("Wt-valid", true);
      result = true;
    }
  }

  if (auth_.attemptThrottlingEnabled())
    loginThrottle(throttlingDelay);

  return result;
}

  }
}
