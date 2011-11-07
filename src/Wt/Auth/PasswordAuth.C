/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/BaseAuth"
#include "Wt/Auth/PasswordAuth"
#include "Wt/Auth/User"
#include "Wt/Mail/Message"
#include "Wt/WRandom"

/*
 * Global throttling:
 *  - per process
 */
namespace Wt {
  namespace Auth {

PasswordAuth::AbstractVerifier::~AbstractVerifier()
{ }

PasswordAuth::AbstractStrengthChecker::~AbstractStrengthChecker()
{ }

PasswordAuth::PasswordAuth(const BaseAuth& baseAuth)
  : baseAuth_(baseAuth),
    verifier_(0),
    checker_(0),
    attemptThrottling_(false)
{ }

PasswordAuth::~PasswordAuth()
{
  delete verifier_;
}

void PasswordAuth::setVerifier(AbstractVerifier *verifier)
{
  delete verifier_;
  verifier_ = verifier;
}

void PasswordAuth::setAttemptThrottlingEnabled(bool enabled)
{
  attemptThrottling_ = enabled;
}

WString PasswordAuth::validatePassword(const WT_USTRING& pwd) const
{
  // FIXME
  if (pwd.toUTF8().length() < 8)
    return WString::tr("Wt.Auth.password-tooshort").arg(8);
  else
    return WString::Empty;
}

int PasswordAuth::delayForNextAttempt(const User& user) const
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

int PasswordAuth::getPasswordThrottle(int failedAttempts) const
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

PasswordResult PasswordAuth::verifyPassword(const User& user,
					    const WT_USTRING& password) const
{
  std::auto_ptr<AbstractUserDatabase::Transaction> t
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

void PasswordAuth::updatePassword(const User& user, const WT_USTRING& password)
  const
{
  PasswordHash pwd = verifier_->hashPassword(password);
  user.setPassword(pwd);
}

  }
}
