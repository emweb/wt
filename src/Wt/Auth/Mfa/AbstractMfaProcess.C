#include "Wt/Auth/Mfa/AbstractMfaProcess.h"

#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Identity.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/User.h"

#include "Wt/Http/Cookie.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WInteractWidget.h"
#include "Wt/WLogger.h"

#include <chrono>

namespace Wt {
  LOGGER("Auth.Mfa.AbstractMfaProcess");

  namespace Auth {
    namespace Mfa {
AuthenticationResult::AuthenticationResult()
  : status_(AuthenticationStatus::Failure)
{
}
AuthenticationResult::AuthenticationResult(AuthenticationStatus status, const Wt::WString& message)
  : status_(status),
    message_(message)
{
}

AuthenticationResult::AuthenticationResult(AuthenticationStatus status)
  : status_(status)
{
}

AbstractMfaProcess::AbstractMfaProcess(const AuthService& authService, AbstractUserDatabase& users, Login& login)
  : throttlingDelay_(0),
    baseAuth_(authService),
    users_(users),
    login_(login)
{
}

AbstractMfaProcess::~AbstractMfaProcess()
{
}

const std::string& AbstractMfaProcess::provider() const
{
  return baseAuth_.mfaProvider();
}

Wt::WString AbstractMfaProcess::userIdentity()
{
  auto currentIdentityValue = login().user().identity(provider());
  if (currentIdentityValue.empty()) {
    LOG_WARN("userIdentity: No identity value for the provider was found. (provider = '" << provider() << "').");
  }
  LOG_DEBUG("userIdentity: A valid identity for the provider was found. (provider = '" << provider() << "').");
  return currentIdentityValue;
}

bool AbstractMfaProcess::createUserIdentity(const Wt::WString& identityValue)
{
  User user = login().user();
  auto currentIdentityValue = user.identity(provider());
  if (!currentIdentityValue.empty()) {
    LOG_WARN("createUserIdentity: The value for the identity was not empty. This identity was not changed.");
    return false;
  }

  user.addIdentity(provider(), identityValue);
  LOG_INFO("createUserIdentity: Adding new identity for provider; " << provider());
  return true;
}

void AbstractMfaProcess::processEnvironment()
{
  User user = processMfaToken();
  if (user.isValid()) {
    login().login(user, LoginState::Weak);
    return;
  }
}

User AbstractMfaProcess::processMfaToken()
{
  WApplication *app = WApplication::instance();
  const WEnvironment& env = app->environment();

  if (baseAuth().authTokensEnabled()) {
    const std::string *token = env.getCookie(baseAuth().mfaTokenCookieName());

    if (token) {
      LOG_INFO("processMfaToken: Processing auth token for MFA for user: " << login().user().id());
      AuthTokenResult result = baseAuth().processAuthToken(*token, users(), AuthTokenType::MFA);

      switch(result.state()) {
        case AuthTokenState::Valid: {
          LOG_INFO("processMfaToken: Found valid match for auth token for MFA for user: " << login().user().id());
          if (!result.newToken().empty()) {
            LOG_DEBUG("processMfaToken: Renewing auth token for MFA.");
            app->setCookie(baseAuth().mfaTokenCookieName(),
                           result.newToken(),
                           result.newTokenValidity(),
                           baseAuth().mfaTokenCookieDomain(),
                           "",
                           app->environment().urlScheme() == "https");
          }

          // Has correct MFA provider?
          auto identity = result.user().identity(provider());
          if (!identity.empty()) {
            // Has matched correct user? Can occur when multiple users use the same device.
            if (login().user() == result.user()) {
              LOG_DEBUG("processMfaToken: Found token with matching user to previous login action.");
              return result.user();
            } else {
              LOG_DEBUG("processMfaToken: Found token with DIFFERENT user to previous login action. This can occur when multiple users use the same device.");
              return User();
            }
          } else {
            LOG_DEBUG("Found no valid identity for the provider: " << provider());
            return User();
          }
        }
        case AuthTokenState::Invalid: {
          LOG_INFO("processMfaToken: Found no valid match for auth token for MFA for user:" << login().user().id() << ", removing token");
          app->setCookie(baseAuth().mfaTokenCookieName(),
                         std::string(),
                         0,
                         baseAuth().mfaTokenCookieDomain(),
                         "",
                         app->environment().urlScheme() == "https");

          return User();
        }
      }
    }
  }

  return User();
}

void AbstractMfaProcess::setRememberMeCookie(User user)
{
  WApplication *app = WApplication::instance();

  int duration = baseAuth().mfaTokenValidity() * 60;

  LOG_INFO("Setting auth token for MFA named: " << baseAuth().mfaTokenCookieName() << " with validity (in seconds): " << duration);
  auto cookie = Http::Cookie(baseAuth().mfaTokenCookieName(),
                             baseAuth().createAuthToken(user, AuthTokenType::MFA));
#ifndef WT_TARGET_JAVA
  cookie.setMaxAge(std::chrono::seconds(duration));
#else
  cookie.setMaxAge(duration);
#endif // WT_TARGET_JAVA
  cookie.setDomain(baseAuth().mfaTokenCookieDomain());
  cookie.setSecure(app->environment().urlScheme() == "https");
  app->setCookie(cookie);
}

void AbstractMfaProcess::setMfaThrottle(std::unique_ptr<AuthThrottle> authThrottle)
{
  mfaThrottle_ = std::move(authThrottle);
}

void AbstractMfaProcess::configureThrottling(WInteractWidget* button)
{
  if (mfaThrottle()) {
    mfaThrottle_->initializeThrottlingMessage(button);
  }
}

void AbstractMfaProcess::updateThrottling(WInteractWidget* button)
{
  if (mfaThrottle()) {
    mfaThrottle_->updateThrottlingMessage(button, throttlingDelay_);
  }
}
    }
  }
}
