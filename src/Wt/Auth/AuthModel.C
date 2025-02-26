/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService.h"
#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/AuthModel.h"
#include "Wt/Auth/HashFunction.h"

#include "Wt/Http/Cookie.h"
#include "Wt/Http/SecureCookie.h"
#include "Wt/Http/HostCookie.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WInteractWidget.h"
#include "Wt/WLogger.h"

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
    passwordAuth()->passwordThrottle()->initializeThrottlingMessage(button);
  }
}

void AuthModel::updateThrottling(WInteractWidget *button)
{
  if (passwordAuth() && passwordAuth()->attemptThrottlingEnabled()) {
    passwordAuth()->passwordThrottle()->updateThrottlingMessage(button, throttlingDelay_);
  }
}

bool AuthModel::validateField(Field field)
{
  if (field == RememberMeField)
    return true;

  User user = users().findWithIdentity(Identity::LoginName,
                                       valueText(LoginNameField));
  if (field == LoginNameField) {
    if (user.isValid()) {
      if (baseAuth()->emailVerificationRequired() &&
        user.email().empty())
      setValidation
	(LoginNameField,
	 WValidator::Result(ValidationState::Invalid,
			    WString::tr("Wt.Auth.email-unverified")));
      else
        setValid(LoginNameField);
    } else {
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

        if (passwordAuth()->attemptThrottlingEnabled()) {
          throttlingDelay_ = passwordAuth()->delayForNextAttempt(user);
        }

        return false;
      case PasswordResult::LoginThrottling:
        setValidation
          (PasswordField,
           WValidator::Result(ValidationState::Invalid,
                              WString::tr("Wt.Auth.password-info")));
        setValidated(PasswordField, false);

        throttlingDelay_ = passwordAuth()->delayForNextAttempt(user);

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

  int duration = baseAuth()->authTokenValidity() * 60;
  const std::string& token = baseAuth()->createAuthToken(user);

  app->setCookie(createAuthCookie(token, duration));
}

bool AuthModel::login(Login& login)
{
  if (valid()) {
#ifndef WT_TARGET_JAVA
    // self: keep model alive until function returns
    // loginUser can cause Login::changed() signal
    // to be emitted. A slot may be connected that
    // deletes the widget that is the sole owner of this model
    auto self = shared_from_this();
#else // WT_TARGET_JAVA
    auto self = this;
#endif // WT_TARGET_JAVA
    User user = users().findWithIdentity(Identity::LoginName,
                                         valueText(LoginNameField));
    cpp17::any v = value(RememberMeField);
    LoginState state = LoginState::Strong;
    // Either MFA is enabled and required, or it is enabled and the user
    // has a secret key attached to it.
    WString totpSecretKey;
    if (user.isValid()) {
      totpSecretKey = user.identity(baseAuth()->mfaProvider());
    }

    if (hasMfaStep(user)) {
      state = LoginState::RequiresMfa;
    }

    if (loginUser(login, user, state)) {
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

      // Retrieve current cookie, if it is present.
      // This retrieves it from the environment. Which is only constructed once, and does NOT update.
      Http::Cookie cookie = createAuthCookie("", 0);
      const std::string* token = app->environment().getCookie(cookie.name());
      // Ensure cookies added by the application during its lifetime are also scanned.
      const std::string* addedToken = app->findAddedCookies(cookie.name());

      std::unique_ptr<AbstractUserDatabase::Transaction> t(users().startTransaction());
      if (token) {
        std::string hash = baseAuth()->tokenHashFunction()->compute(*token, std::string());
        users().removeAuthToken(login.user(), hash);
      }

      if (addedToken) {
        std::string hash = baseAuth()->tokenHashFunction()->compute(*addedToken, std::string());
        users().removeAuthToken(login.user(), hash);
      }

      if (t.get()) {
        t->commit();
      }

      // Mark the cookie to be removed
      app->removeCookie(cookie);
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
    AuthCookiePrefix prefix = baseAuth()->authTokenCookiePrefix();
    const std::string *token = nullptr;
    const std::string &cookieName = baseAuth()->authTokenCookieName();

    switch (prefix) {
      case AuthCookiePrefix::Empty:
        token = env.getCookie(cookieName);
        break;
      case AuthCookiePrefix::Secure:
        token = env.getSecureCookie(cookieName);
        break;
      case AuthCookiePrefix::Host:
        token = env.getHostCookie(cookieName);
        break;
      default:
        break;
    }

    if (token) {
      AuthTokenResult result = baseAuth()->processAuthToken(*token, users());

      switch(result.state()) {
      case AuthTokenState::Valid: {
        if (!result.newToken().empty()) {
          /*
           * Only extend the validity from what we had currently.
           */
          Http::Cookie cookie = createAuthCookie(result.newToken(), result.newTokenValidity());
          cookie.setDomain("");
          app->setCookie(cookie);
        }

        return result.user();
      }
      case AuthTokenState::Invalid:
        Http::Cookie cookie = createAuthCookie("", 0);
        cookie.setDomain("");
        app->setCookie(cookie);

        return User();
      }
    }
  }

  return User();
}

bool AuthModel::showResendEmailVerification() const
{
  if (!baseAuth()->emailVerificationRequired()) {
    return false;
  }

  User user = users().findWithIdentity(Identity::LoginName,
                                       valueText(LoginNameField));
  return user.isValid() && user.email().empty();
}

bool AuthModel::hasMfaStep(const User& user) const
{
  WString totpSecretKey;
  if (user.isValid()) {
    totpSecretKey = user.identity(baseAuth()->mfaProvider());
  }

  if (baseAuth()->mfaEnabled() && !baseAuth()->mfaRequired() &&totpSecretKey.empty()) {
    LOG_WARN("hasMfaStep(): MFA is enabled (but not required), and no valid identity was found. The MFA step is skipped for user: " << user.id());
  }

  return ((baseAuth()->mfaEnabled() && !totpSecretKey.empty())
          || (baseAuth()->mfaEnabled() && baseAuth()->mfaRequired()));

}

Http::Cookie AuthModel::createAuthCookie(const std::string& value,
                                         int duration) const
{
  WApplication *app = WApplication::instance();

  AuthCookiePrefix prefix = baseAuth()->authTokenCookiePrefix();
  const std::string &cookieName = baseAuth()->authTokenCookieName();

  Http::Cookie cookie(cookieName);
  switch (prefix) {
  case AuthCookiePrefix::Secure:
    cookie = Http::SecureCookie(cookieName);
    break;
  case AuthCookiePrefix::Host:
    cookie = Http::HostCookie(cookieName);
    break;
  default:
    break;
  }
  
  switch (prefix) {
  case AuthCookiePrefix::Empty:
    cookie.setSecure(app->environment().urlScheme() == "https");
    WT_FALLTHROUGH
  case AuthCookiePrefix::Secure:
    cookie.setDomain(baseAuth()->authTokenCookieDomain());
    break;
  default:
    break;
  }

  cookie.setValue(value);

#ifndef WT_TARGET_JAVA
  cookie.setMaxAge(std::chrono::seconds(duration));
#else
  cookie.setMaxAge(duration);
#endif // WT_TARGET_JAVA

  return cookie;
}
  }
}
