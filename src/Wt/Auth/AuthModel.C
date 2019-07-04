/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService.h"
#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/AuthModel.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WInteractWidget.h"
#include "Wt/WLogger.h"

#ifndef WT_DEBUG_JS
#include "js/AuthModel.min.js"
#endif

#include "Wt/WDllDefs.h"

#include <memory>

namespace Wt {

LOGGER("Auth.AuthModel");

  namespace Auth {

const WFormModel::Field AuthModel::PasswordField = "password";
const WFormModel::Field AuthModel::RememberMeField = "remember-me";

AuthModel::AuthModel(const AuthService& baseAuth, AbstractUserDatabase& users)
  : FormBaseModel(baseAuth, users),
    throttlingDelay_(0)
{
  reset();
}

void AuthModel::reset()
{
  if (baseAuth()->identityPolicy() == IdentityPolicy::EmailAddress)
    addField(LoginNameField, WString::tr("Wt.Auth.email-info"));
  else
    addField(LoginNameField, WString::tr("Wt.Auth.user-name-info"));

  addField(PasswordField, WString::tr("Wt.Auth.password-info"));

  int days = baseAuth()->authTokenValidity() / 24 / 60;

  WString info;
  if (days % 7 != 0)
    info = WString::trn("Wt.Auth.remember-me-info.days", days).arg(days);
  else
    info = WString::trn("Wt.Auth.remember-me-info.weeks", days/7).arg(days/7);

  addField(RememberMeField, info);
  setValidation(RememberMeField, WValidator::Result(ValidationState::Valid,
						    info));
}

bool AuthModel::isVisible(Field field) const
{
  if (field == RememberMeField)
    return baseAuth()->authTokensEnabled();
  else
    return WFormModel::isVisible(field);
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
    s << button->jsRef() << ".wtThrottle.reset("
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
	 WValidator::Result(ValidationState::Invalid,
			    WString::tr("Wt.Auth.user-name-invalid")));

      throttlingDelay_ = 0;
    }

    return user.isValid();
  } else if (field == PasswordField) {
    if (user.isValid()) {
      PasswordResult r
	= passwordAuth()->verifyPassword(user, valueText(PasswordField));

      switch (r) {
      case PasswordResult::PasswordInvalid:
	setValidation
	  (PasswordField,
	   WValidator::Result(ValidationState::Invalid,
			      WString::tr("Wt.Auth.password-invalid")));

	if (passwordAuth()->attemptThrottlingEnabled())
	  throttlingDelay_ = passwordAuth()->delayForNextAttempt(user);

	return false;
      case PasswordResult::LoginThrottling:
	setValidation
	  (PasswordField,
	   WValidator::Result(ValidationState::Invalid,
			      WString::tr("Wt.Auth.password-info")));
	setValidated(PasswordField, false);

	throttlingDelay_ = passwordAuth()->delayForNextAttempt(user);
	LOG_SECURE("throttling: " << throttlingDelay_
		   << " seconds for " << user.identity(Identity::LoginName));

	return false;
      case PasswordResult::PasswordValid:
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
  std::unique_ptr<AbstractUserDatabase::Transaction>
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
    // self: keep model alive until function returns
    // loginUser can cause Login::changed() signal
    // to be emitted. A slot may be connected that
    // deletes the widget that is the sole owner of this model
    auto self = shared_from_this();
    User user = users().findWithIdentity(Identity::LoginName,
					 valueText(LoginNameField));
    cpp17::any v = value(RememberMeField);
    if (loginUser(login, user)) {
      reset();

      if (cpp17::any_has_value(v) && cpp17::any_cast<bool>(v) == true)
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
      env.getCookie(baseAuth()->authTokenCookieName());

    if (token) {
      AuthTokenResult result = baseAuth()->processAuthToken(*token, users());

      switch(result.state()) {
      case AuthTokenState::Valid: {
        if (!result.newToken().empty()) {
          /*
           * Only extend the validity from what we had currently.
           */
          app->setCookie(baseAuth()->authTokenCookieName(), result.newToken(),
                         result.newTokenValidity(), "", "", app->environment().urlScheme() == "https");
        }

	return result.user();
      }
      case AuthTokenState::Invalid:
        app->setCookie(baseAuth()->authTokenCookieName(),std::string(), 0, "", "", app->environment().urlScheme() == "https");

	return User();
      }
    }
  }

  return User();
}

  }
}
