/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/User"

#include "Wt/WDllDefs.h"

#include <memory>

#ifdef WT_CXX11
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

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
    verifier_(0),
    validator_(0),
    attemptThrottling_(false)
{ }

PasswordService::~PasswordService()
{
  delete verifier_;
  delete validator_;
}

void PasswordService::setVerifier(AbstractVerifier *verifier)
{
  delete verifier_;
  verifier_ = verifier;
}

void PasswordService::setStrengthValidator(AbstractStrengthValidator *validator)
{
  delete validator_;
  validator_ = validator;
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
  AUTO_PTR<AbstractUserDatabase::Transaction> t
    (user.database()->startTransaction());

  if (delayForNextAttempt(user) > 0)
    return LoginThrottling;

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

    return PasswordValid;
  } else {
    if (t.get())
      t->commit();

    return PasswordInvalid;
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
