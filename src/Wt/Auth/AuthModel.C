/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/Login"
#include "Wt/Auth/AuthModel"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WInteractWidget"
#include "Wt/WLogger"

#ifndef WT_DEBUG_JS
#include "js/AuthModel.min.js"
#endif

#include "Wt/WDllDefs.h"

#include <memory>

#ifdef WT_CXX11
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

namespace Wt {

LOGGER("Auth.AuthModel");

  namespace Auth {

const WFormModel::Field AuthModel::PasswordField = "password";
const WFormModel::Field AuthModel::RememberMeField = "remember-me";

AuthModel::AuthModel(const AuthService& baseAuth, AbstractUserDatabase& users,
		     WObject *parent)
  : FormBaseModel(baseAuth, users, parent),
    throttlingDelay_(0)
{
  reset();
}

void AuthModel::reset()
{
  if (baseAuth()->identityPolicy() == EmailAddressIdentity)
    addField(LoginNameField, WString::tr("Wt.Auth.email-info"));
  else
    addField(LoginNameField, WString::tr("Wt.Auth.user-name-info"));

  addField(PasswordField, WString::tr("Wt.Auth.password-info"));

  int days = baseAuth()->authTokenValidity() / 24 / 60;

  WString info;
  if (days % 7 != 0)
    info = WString::tr("Wt.Auth.remember-me-info.days").arg(days);
  else
    info = WString::tr("Wt.Auth.remember-me-info.weeks").arg(days/7);

  addField(RememberMeField, info);
  setValidation(RememberMeField, WValidator::Result(WValidator::Valid, info));
}

bool AuthModel::isVisible(Field field) const
{
  if (field == RememberMeField)
    return baseAuth()->authTokensEnabled();
  else
    return true;
}

void AuthModel::configureThrottling(WInteractWidget *button)
{
  if (passwordAuth() && passwordAuth()->attemptThrottlingEnabled()) {
    WApplication *app = WApplication::instance();
    LOAD_JAVASCRIPT(app, "js/AuthModel.js", "AuthThrottle", wtjs1);

    button->setJavaScriptMember(" AuthThrottle",
				"new " WT_CLASS ".AuthThrottle(" WT_CLASS ","
				+ button->jsRef() + ","
				+ WString::tr("Wt.Auth.throttle-retry")
				.jsStringLiteral()
				+ ");");
  }
}

void AuthModel::updateThrottling(WInteractWidget *button)
{
  if (passwordAuth() && passwordAuth()->attemptThrottlingEnabled()) {
    WStringStream s;
    s << "jQuery.data(" << button->jsRef() << ", 'throttle').reset("
      << throttlingDelay_ << ");";

    button->doJavaScript(s.str());
  }
}

bool AuthModel::validateField(Field field)
{
  if (field == RememberMeField)
    return true;

  User user = users().findWithIdentity(Identity::LoginName,
				       valueText(LoginNameField));
  if (field == LoginNameField) {
    if (user.isValid())
      setValid(LoginNameField);
    else {
      setValidation
	(LoginNameField,
	 WValidator::Result(WValidator::Invalid,
			    WString::tr("Wt.Auth.user-name-invalid")));

      throttlingDelay_ = 0;
    }

    return user.isValid();
  } else if (field == PasswordField) {
    if (user.isValid()) {
      PasswordResult r
	= passwordAuth()->verifyPassword(user, valueText(PasswordField));

      switch (r) {
      case PasswordInvalid:
	setValidation
	  (PasswordField,
	   WValidator::Result(WValidator::Invalid,
			      WString::tr("Wt.Auth.password-invalid")));

	if (passwordAuth()->attemptThrottlingEnabled())
	  throttlingDelay_ = passwordAuth()->delayForNextAttempt(user);

	return false;
      case LoginThrottling:
	setValidation
	  (PasswordField,
	   WValidator::Result(WValidator::Invalid,
			      WString::tr("Wt.Auth.password-info")));
	setValidated(PasswordField, false);

	throttlingDelay_ = passwordAuth()->delayForNextAttempt(user);
	LOG_SECURE("throttling: " << throttlingDelay_
		   << " seconds for " << user.identity(Identity::LoginName));

	return false;
      case PasswordValid:
	setValid(PasswordField);
	return true;
      }

      /* unreachable */
      return false;
    } else
      return false;
  } else
    return false;
}

bool AuthModel::validate()
{
  AUTO_PTR<AbstractUserDatabase::Transaction>
    t(users().startTransaction());

  bool result = WFormModel::validate();

  if (t.get())
    t->commit();

  return result;
}

void AuthModel::setRememberMeCookie(const User& user)
{
  WApplication *app = WApplication::instance();
  const AuthService *s = baseAuth();

  app->setCookie(s->authTokenCookieName(),
		 s->createAuthToken(user),
		 s->authTokenValidity() * 60,
		 s->authTokenCookieDomain(),
		 "",
		 app->environment().urlScheme() == "https");
}

bool AuthModel::login(Login& login)
{
  if (valid()) {
    User user = users().findWithIdentity(Identity::LoginName,
					 valueText(LoginNameField));
    boost::any v = value(RememberMeField);
    if (loginUser(login, user)) {
      reset();

      if (!v.empty() && boost::any_cast<bool>(v) == true)
	setRememberMeCookie(user);

      return true;
    } else
      return false;
  } else
    return false;
}

void AuthModel::logout(Login& login)
{
  if (login.loggedIn()) {
    if (baseAuth()->authTokensEnabled()) {
      WApplication *app = WApplication::instance();
      app->removeCookie(baseAuth()->authTokenCookieName());

      /*
       * FIXME: it would be nice if we also delete the relevant token
       * from the database!
       */
    }

    login.logout();
  }
}

EmailTokenResult AuthModel::processEmailToken(const std::string& token)
{
  return baseAuth()->processEmailToken(token, users());
}

User AuthModel::processAuthToken()
{
  WApplication *app = WApplication::instance();
  const WEnvironment& env = app->environment();

  if (baseAuth()->authTokensEnabled()) {
    const std::string *token =
      env.getCookieValue(baseAuth()->authTokenCookieName());

    if (token) {
      AuthTokenResult result = baseAuth()->processAuthToken(*token, users());

      switch(result.result()) {
      case AuthTokenResult::Valid:
	/*
	 * Only extend the validity from what we had currently.
	 */
	app->setCookie(baseAuth()->authTokenCookieName(), result.newToken(),
		       result.newTokenValidity(), "", "", app->environment().urlScheme() == "https");

	return result.user();
      case AuthTokenResult::Invalid:
	app->setCookie(baseAuth()->authTokenCookieName(),std::string(), 0, "", "", app->environment().urlScheme() == "https");

	return User();
      }
    }
  }

  return User();
}

  }
}
