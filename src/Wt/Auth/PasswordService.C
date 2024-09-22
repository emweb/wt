/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/User.h"

#include "Wt/WDllDefs.h"

#include <memory>

/*
 * Global throttling:
 *  - per process
 */
namespace Wt {
  namespace Auth {

PasswordService::AbstractVerifier::~AbstractVerifier()
{ }

PasswordService::PasswordService(const AuthService& baseAuth)
  : baseAuth_(baseAuth)
{ }

PasswordService::~PasswordService()
{ }

void PasswordService::setVerifier(std::unique_ptr<AbstractVerifier> verifier)
{
  verifier_ = std::move(verifier);
}

void PasswordService
::setStrengthValidator(std::unique_ptr<AbstractStrengthValidator> validator)
{
  validator_ = std::move(validator);
}

void PasswordService::setPasswordThrottle(std::unique_ptr<AuthThrottle> delayer)
{
  passwordThrottle_ = std::move(delayer);
}

void PasswordService::setAttemptThrottlingEnabled(bool enabled)
{
  if(enabled) {
    passwordThrottle_.reset(new AuthThrottle());
  } else {
    passwordThrottle_.reset(nullptr);
  }
}

int PasswordService::delayForNextAttempt(const User& user) const
{
  if (passwordThrottle()) {
    return passwordThrottle()->delayForNextAttempt(user);
  }

  return 0;
}

int PasswordService::getAuthenticationThrottle(int failedAttempts) const
{
  if (passwordThrottle()) {
    return passwordThrottle()->getAuthenticationThrottle(failedAttempts);
  }

  return 0;
}

PasswordResult PasswordService::verifyPassword(const User& user,
                                               const WT_USTRING& password) const
{
  std::unique_ptr<AbstractUserDatabase::Transaction> t
    (user.database()->startTransaction());

  if (delayForNextAttempt(user) > 0)
    return PasswordResult::LoginThrottling;

  bool valid = verifier_->verify(password, user.password());

  if (passwordThrottle()) {
    user.setAuthenticated(valid); // XXX rename to .passwordAttempt()
  }

  if (valid) {
    /*
     * Upgrade its password if needed.
     */
    if (verifier_->needsUpdate(user.password()))
      user.setPassword(verifier_->hashPassword(password));

    if (t.get())
      t->commit();

    return PasswordResult::PasswordValid;
  } else {
    if (t.get())
      t->commit();

    return PasswordResult::PasswordInvalid;
  }
}

void PasswordService::updatePassword(const User& user,
                                     const WT_USTRING& password) const
{
  PasswordHash pwd = verifier_->hashPassword(password);
  user.setPassword(pwd);
}

  }
}
