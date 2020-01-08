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
  : baseAuth_(baseAuth),
    attemptThrottling_(false)
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

void PasswordService::setAttemptThrottlingEnabled(bool enabled)
{
  attemptThrottling_ = enabled;
}

int PasswordService::delayForNextAttempt(const User& user) const
{
  if (attemptThrottling_) {
    int throttlingNeeded = getPasswordThrottle(user.failedLoginAttempts());

    if (throttlingNeeded) {
      WDateTime t = user.lastLoginAttempt();
      int diff = t.secsTo(WDateTime::currentDateTime());

      if (diff < throttlingNeeded)
	return throttlingNeeded - diff;
      else
	return 0;
    } else
	return 0;
  } else
    return 0;
}

int PasswordService::getPasswordThrottle(int failedAttempts) const
{
  switch (failedAttempts) {
  case 0:
    return 0;
  case 1:
    return 1;
  case 2:
    return 5;
  case 3:
    return 10;
  default:
    return 25;
  }
}

PasswordResult PasswordService::verifyPassword(const User& user,
					    const WT_USTRING& password) const
{
  std::unique_ptr<AbstractUserDatabase::Transaction> t
    (user.database()->startTransaction());

  if (delayForNextAttempt(user) > 0)
    return PasswordResult::LoginThrottling;

  bool valid = verifier_->verify(password, user.password());

  if (attemptThrottling_)
    user.setAuthenticated(valid); // XXX rename to .passwordAttempt()

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
