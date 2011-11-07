/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/BaseAuth"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/User"
#include "Wt/Mail/Client"
#include "Wt/Mail/Message"
#include "Wt/WApplication"
#include "Wt/WRandom"

#include "web/Utils.h"

namespace Wt {
  namespace Auth {

/*! \defgroup auth Authentication module (Wt::Auth)
 *  \brief A module that implements authentication functions.
 *
 * This module implements a complete modular authentication system for
 * %Wt applications.
 *
 * The module is organized in model classes, which implement
 * authentication logic, and view classes which are widgets that
 * implement UI components.
 *
 * <h3>Model</h3>
 *
 * The model layer is organized in a number of service classes, which
 * provide the configuration and authentication services, and are
 * generally shared between different sessions, and session classes
 * which implement the authentication state for each session.
 *
 * The <b>service classes</b> are:
 * - BaseAuth
 * - PasswordAuth
 * - OAuth (with its provider-specific implementations)
 *
 * These classes use a number of utility model classes:
 * - User
 * - Token
 * - PasswordHash
 * - PasswordStrengthChecker
 * - PasswordVerifier
 *
 * The <b>session classes</b> are:
 * - AbstractUserDatabase, for which you will need to define a concrete
 *   implementation
 * - Login
 * - OAuth::Process
 *
 * <h3>Views</h3>
 *
 * The view classes use abstract service classes.
 *
 * The included views are:
 * - AuthWidget
 * - LostPasswordWidget
 * - PasswordPromptDialog
 * - RegistrationWidget
 * - UpdatePasswordWidget
 */

EmailTokenResult::EmailTokenResult(Result result, const User& user)
  : result_(result),
    user_(user)
{ }

const User& EmailTokenResult::user() const
{
  if (user_.isValid())
    return user_;
  else
    throw WException("EmailTokenResult::user() invalid");
}

AuthTokenResult::AuthTokenResult(Result result, const User& user,
				 const std::string& newToken)
  : result_(result),
    user_(user),
    newToken_(newToken)
{ }

const User& AuthTokenResult::user() const
{
  if (user_.isValid())
    return user_;
  else
    throw WException("AuthTokenResult::user() invalid");
}

std::string AuthTokenResult::newToken() const
{
  if (user_.isValid())
    return newToken_;
  else
    throw WException("AuthTokenResult::newToken() invalid");
}

BaseAuth::BaseAuth()
  : minimumIdentityLength_(8),
    tokenHashFunction_(0),
    tokenLength_(32),
    emailVerification_(false),
    emailTokenValidity_(3 * 24 * 60),  // three days
    authTokens_(false),
    authTokenValidity_(14 * 24 * 60)   // two weeks
{ }

BaseAuth::~BaseAuth()
{
  delete tokenHashFunction_;
}

void BaseAuth::setEmailVerificationEnabled(bool enabled)
{
  emailVerification_ = enabled;
}

void BaseAuth::setAuthTokensEnabled(bool enabled, const std::string& cookieName)
{
  authTokens_ = enabled;
  authTokenCookieName_ = cookieName;
}

WString BaseAuth::validateIdentity(const WT_USTRING& identity) const
{
  if (static_cast<int>(identity.toUTF8().length()) < minimumIdentityLength_)
    return WString::tr("Wt.Auth.identity-tooshort")
      .arg(minimumIdentityLength_);
  else
    return WString::Empty;
}

void BaseAuth::setTokenHashFunction(const HashFunction *function)
{
  delete tokenHashFunction_;
  tokenHashFunction_ = function;
}

const HashFunction *BaseAuth::tokenHashFunction() const
{
  if (tokenHashFunction_)
    return tokenHashFunction_;
  else
    throw WException("Auth: don't have a token hash function");
}

std::string BaseAuth::createAuthToken(const User& user) const
{
  if (!user.isValid())
    throw WException("Auth: createAuthToken(): user invalid");

  std::auto_ptr<AbstractUserDatabase::Transaction>
    t(user.database()->startTransaction());

  std::string random = WRandom::generateId(tokenLength_);
  std::string hash = tokenHashFunction()->compute(random, std::string());

  Token token
    (hash, WDateTime::currentDateTime().addSecs(authTokenValidity_ * 60));
  user.addAuthToken(token);

  if (t.get()) t->commit();

  return random;
}

AuthTokenResult BaseAuth::processAuthToken(const std::string& token,
					   AbstractUserDatabase& users) const
{
  std::auto_ptr<AbstractUserDatabase::Transaction> t(users.startTransaction());

  std::string hash = tokenHashFunction()->compute(token, std::string());

  User user = users.findWithAuthToken(hash);

  if (user.isValid()) {
    user.removeAuthToken(hash);

    std::string newToken = createAuthToken(user);

    if (t.get()) t->commit();

    return AuthTokenResult(AuthTokenResult::Valid, user, newToken);
  } else
    return AuthTokenResult(AuthTokenResult::Invalid);
}

void BaseAuth::verifyEmailAddress(const User& user, const std::string& address)
  const
{
  user.setUnverifiedEmail(address);

  std::string random = WRandom::generateId(tokenLength_);
  std::string hash = tokenHashFunction()->compute(random, std::string());

  Token t(hash,
	  WDateTime::currentDateTime().addSecs(emailTokenValidity_ * 60));
  user.setEmailToken(t, User::VerifyEmail);
  sendConfirmMail(address, user, random);
}

void BaseAuth::lostPassword(const std::string& emailAddress,
			    AbstractUserDatabase& users) const
{
  /*
   * This will check that a user exists in the database, and if so,
   * send an email.
   */
  User user = users.findWithEmail(emailAddress);

  if (user.isValid()) {
    std::string random = WRandom::generateId(randomTokenLength());
    std::string hash = tokenHashFunction()->compute(random, std::string());

    WDateTime expires = WDateTime::currentDateTime();
    expires = expires.addSecs(emailTokenValidity() * 60);

    Token t(hash, expires);
    user.setEmailToken(t, User::LostPassword);
    sendLostPasswordMail(emailAddress, user, random);
  }
}

std::string BaseAuth::parseEmailToken(const std::string& internalPath) const
{
  std::string ip = Utils::append(internalPath, '/');

  if (WApplication::pathMatches(ip, redirectInternalPath_)) {
    return ip.substr(redirectInternalPath_.length());
  } else
    return std::string();
}

EmailTokenResult BaseAuth::processEmailToken(const std::string& token,
					     AbstractUserDatabase& users) const
{
  std::auto_ptr<AbstractUserDatabase::Transaction> tr(users.startTransaction());

  User user = users.findWithEmailToken(token);

  if (user.isValid()) {
    Token t = user.emailToken();

    if (t.expirationTime() < WDateTime::currentDateTime()) {
      user.clearEmailToken();

      if (tr.get())
	tr->commit();

      return EmailTokenResult::Expired;
    }

    switch (user.emailTokenRole()) {
    case User::LostPassword:
      user.clearEmailToken();

      if (tr.get())
	tr->commit();

      return EmailTokenResult(EmailTokenResult::UpdatePassword, user);

    case User::VerifyEmail:
      user.clearEmailToken();
      user.setEmail(user.unverifiedEmail());
      user.setUnverifiedEmail(std::string());

      if (tr.get())
	tr->commit();

      return EmailTokenResult(EmailTokenResult::EmailConfirmed, user);

    default:
      if (tr.get())
	tr->commit();

      return EmailTokenResult::Invalid; // Unreachable
    }
  } else {
    if (tr.get())
      tr->commit();

    return EmailTokenResult::Invalid;
  }
}


void BaseAuth::sendConfirmMail(const std::string& address,
			       const User& user, const std::string& token) const
{
  Mail::Message message;

  message.addRecipient(Mail::To, Mail::Mailbox(address));
  message.setSubject(WString::tr("Wt.auth.confirmmail.subject"));
  message.setBody(WString::tr("Wt.auth.confirmmail.body")
		  .arg(user.identity()).arg(token));
  message.addHtmlBody(WString::tr("Wt.auth.confirmmail.htmlbody")
		      .arg(user.identity()).arg(token));

  sendMail(message);
}


void BaseAuth::sendLostPasswordMail(const std::string& address,
				    const User& user,
				    const std::string& token) const
{
  Mail::Message message;

  message.addRecipient(Mail::To, Mail::Mailbox(address));
  message.setSubject(WString::tr("Wt.auth.lostpasswordmail.subject"));
  message.setBody(WString::tr("Wt.auth.lostpasswordmail.body")
		  .arg(user.identity()).arg(token));
  message.addHtmlBody(WString::tr("Wt.auth.lostpasswordmail.htmlbody")
		      .arg(user.identity()).arg(token));

  sendMail(message);
}

void BaseAuth::sendMail(const Mail::Message& message) const
{
  Mail::Message m = message;

  if (m.from().empty()) {
    std::string senderName = "Wt::Auth";
    std::string senderAddress = "noreply-auth@www.webtoolkit.eu";

    WApplication::readConfigurationProperty("auth-mail-sender-name",
					    senderName);
    WApplication::readConfigurationProperty("auth-mail-sender-address",
					    senderAddress);

    m.setFrom(Mail::Mailbox(senderAddress, WString::fromUTF8(senderName)));
  }

  Mail::Client client;
  client.connect();
  client.send(message);
}

  }
}
