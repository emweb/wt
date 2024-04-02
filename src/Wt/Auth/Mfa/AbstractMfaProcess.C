#include "Wt/Auth/Mfa/AbstractMfaProcess.h"

#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Identity.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/User.h"

#include "Wt/WLogger.h"

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
  : baseAuth_(authService),
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

  std::unique_ptr<AbstractUserDatabase::Transaction> t(users().startTransaction());
  user.addIdentity(provider(), identityValue);
  LOG_INFO("createUserIdentity: Adding new identity for provider: " << provider());

  if (t.get()) {
    t->commit();
  }
  return true;
}
}
  }
}
