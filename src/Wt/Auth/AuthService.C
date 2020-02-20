/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/Identity.h"
#include "Wt/Auth/User.h"
#include "Wt/Auth/MailUtils.h"
#include "Wt/Mail/Client.h"
#include "Wt/Mail/Message.h"
#include "Wt/WApplication.h"
#include "Wt/WRandom.h"

#include "Wt/WDllDefs.h"

#include <memory>

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
 * - AuthService: implements generic authentication services
 * - PasswordService: implements password-based authentication
 * - OAuthService: implements federated login using OAuth 2 (and later
 *   OpenIDConnect), and has a number of indentity provider
 *   implementations
 *
 * There are a number of utility classes too:
 * - User: a user value class
 * - Token: an authentication token
 * - PasswordHash: a hashed password
 * - PasswordStrengthValidator: validates the strength of a password
 * - PasswordVerifier: verifies a password (against a hash stored in the
 *   database)
 *
 * The <b>session classes</b> are:
 * - AbstractUserDatabase: abstract interface for storage needed for
 *   authentication
 * - Login: keeps track of the user currently logged in
 * - OAuthProcess: implements an OAuth 2 authorization (and authentication)
 *   process
 * - RegistrationModel: implements the registration logic
 *
 * <h3>Views</h3>
 *
 * The view classes typically use service classes and session classes.
 *
 * The included views are:
 * - AuthWidget: a (traditional) login/logout widget
 * - LostPasswordWidget: a widget that implements a "lost password" form
 * - PasswordPromptDialog: a dialog to prompt for a password
 * - RegistrationWidget: a widget to register a new user
 * - UpdatePasswordWidget: a widget to update a password
 */

EmailTokenResult::EmailTokenResult(EmailTokenState state, const User& user)
  : state_(state),
    user_(user)
{ }

const User& EmailTokenResult::user() const
{
  if (user_.isValid())
    return user_;
  else
    throw WException("EmailTokenResult::user() invalid");
}

AuthTokenResult::AuthTokenResult(AuthTokenState state, const User& user,
				 const std::string& newToken,
				 int newTokenValidity)
  : state_(state),
    user_(user),
    newToken_(newToken),
    newTokenValidity_(newTokenValidity)
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

int AuthTokenResult::newTokenValidity() const
{
  if (user_.isValid())
    return newTokenValidity_;
  else
    throw WException("AuthTokenResult::newTokenValidity() invalid");
}

AuthService::AuthService()
  : identityPolicy_(IdentityPolicy::LoginName),
    minimumLoginNameLength_(4),
    tokenHashFunction_(new MD5HashFunction()),
    tokenLength_(32),
    emailVerification_(false),
    emailTokenValidity_(3 * 24 * 60),  // three days
    authTokens_(false),
    authTokenUpdateEnabled_(true),
    authTokenValidity_(14 * 24 * 60)   // two weeks
{
  redirectInternalPath_ = "/auth/mail/";
}

AuthService::~AuthService()
{ }

void AuthService::setEmailVerificationEnabled(bool enabled)
{
  emailVerification_ = enabled;
  if (!enabled)
    emailVerificationReq_ = false;
}

void AuthService::setEmailVerificationRequired(bool enabled)
{
  emailVerificationReq_ = enabled;
  if (enabled)
    emailVerification_ = true;
}

void AuthService::setEmailRedirectInternalPath(const std::string& internalPath)
{
  redirectInternalPath_ = internalPath;
}

void AuthService::setIdentityPolicy(IdentityPolicy identityPolicy)
{
  identityPolicy_ = identityPolicy;
}

void AuthService::setAuthTokensEnabled(bool enabled, const std::string& cookieName,
				       const std::string& cookieDomain)
{
  authTokens_ = enabled;
  authTokenCookieName_ = cookieName;
  authTokenCookieDomain_ = cookieDomain;
}

User AuthService::identifyUser(const Identity& identity,
			    AbstractUserDatabase& users) const
{
  std::unique_ptr<AbstractUserDatabase::Transaction> t(users.startTransaction());

  User user = users.findWithIdentity(identity.provider(),
				     WString::fromUTF8(identity.id()));

  if (user.isValid()) {
    if (t.get())
      t->commit();
    return user;
  }

  /*
   * Scenarios to handle with respect to the email address.
   *
   * - in case we are doing email address verification
   *   - existing user has same email address
   *     - new email address is verified -> merge new identity and
   *       simply login as that user [X]
   *     - new email address is not verified -> send email to confirm
   *       to merge the accounts when registring -> TODO
   * - in case we are not doing email address verification
   *   - we'll simply error on a duplicate email address [X]
   */
  if (!identity.email().empty()) {
    if (emailVerification_ && identity.emailVerified()) {
      user = users.findWithEmail(identity.email());
      if (user.isValid()) {
	user.addIdentity(identity.provider(), identity.id());

	if (t.get())
	  t->commit();

	return user;
      }
    }
  }

  if (t.get())
    t->commit();

  return User();
}

void AuthService::setTokenHashFunction(std::unique_ptr<HashFunction> function)
{
  tokenHashFunction_ = std::move(function);
}

HashFunction *AuthService::tokenHashFunction() const
{
  return tokenHashFunction_.get();
}

std::string AuthService::createAuthToken(const User& user) const
{
  if (!user.isValid())
    throw WException("Auth: createAuthToken(): user invalid");

  std::unique_ptr<AbstractUserDatabase::Transaction>
    t(user.database()->startTransaction());

  std::string random = WRandom::generateId(tokenLength_);
  std::string hash = tokenHashFunction()->compute(random, std::string());

  Token token
    (hash, WDateTime::currentDateTime().addSecs(authTokenValidity_ * 60));
  user.addAuthToken(token);

  if (t.get()) t->commit();

  return random;
}

AuthTokenResult AuthService::processAuthToken(const std::string& token,
					      AbstractUserDatabase& users) const
{
  std::unique_ptr<AbstractUserDatabase::Transaction> t(users.startTransaction());

  std::string hash = tokenHashFunction()->compute(token, std::string());

  User user = users.findWithAuthToken(hash);

  if (user.isValid()) {
    if (authTokenUpdateEnabled_) {
      std::string newToken = WRandom::generateId(tokenLength_);
      std::string newHash = tokenHashFunction()->compute(newToken, std::string());
      int validity = user.updateAuthToken(hash, newHash);

      if (validity < 0) {
        /*
         * Old API, this is bad since we always extend the lifetime of the
         * token.
         */
        user.removeAuthToken(hash);
        newToken = createAuthToken(user);
        validity = authTokenValidity_ * 60;
      }

      if (t.get())
        t->commit();

      return AuthTokenResult(AuthTokenState::Valid, user, newToken, validity);
    } else {
      return AuthTokenResult(AuthTokenState::Valid, user);
    }
  } else {
    if (t.get())
      t->commit();
    
    return AuthTokenResult(AuthTokenState::Invalid);
  }
}

void AuthService::verifyEmailAddress(const User& user, const std::string& address)
  const
{
  user.setUnverifiedEmail(address);

  std::string random = WRandom::generateId(tokenLength_);
  std::string hash = tokenHashFunction()->compute(random, std::string());

  Token t(hash,
	  WDateTime::currentDateTime().addSecs(emailTokenValidity_ * 60));
  user.setEmailToken(t, EmailTokenRole::VerifyEmail);
  sendConfirmMail(address, user, random);
}

void AuthService::lostPassword(const std::string& emailAddress,
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
    user.setEmailToken(t, EmailTokenRole::LostPassword);
    sendLostPasswordMail(user.email(), user, random);
  }
}

std::string AuthService::parseEmailToken(const std::string& internalPath) const
{
  if (emailVerification_ && 
      WApplication::pathMatches(internalPath, redirectInternalPath_))
    return internalPath.substr(redirectInternalPath_.length());
  else
    return std::string();
}

EmailTokenResult AuthService::processEmailToken(const std::string& token,
					     AbstractUserDatabase& users) const
{
  std::unique_ptr<AbstractUserDatabase::Transaction> tr(users.startTransaction());

  std::string hash = tokenHashFunction()->compute(token, std::string());

  User user = users.findWithEmailToken(hash);

  if (user.isValid()) {
    Token t = user.emailToken();

    if (t.expirationTime() < WDateTime::currentDateTime()) {
      user.clearEmailToken();

      if (tr.get())
	tr->commit();

      return EmailTokenResult(EmailTokenState::Expired);
    }

    switch (user.emailTokenRole()) {
    case EmailTokenRole::LostPassword:
      if (tr.get())
	tr->commit();

      return EmailTokenResult(EmailTokenState::UpdatePassword, user);

    case EmailTokenRole::VerifyEmail:
      user.clearEmailToken();
      user.setEmail(user.unverifiedEmail());
      user.setUnverifiedEmail(std::string());

      if (tr.get())
	tr->commit();

      return EmailTokenResult(EmailTokenState::EmailConfirmed, user);

    default:
      if (tr.get())
	tr->commit();

      return EmailTokenResult(EmailTokenState::Invalid); // Unreachable
    }
  } else {
    if (tr.get())
      tr->commit();

    return EmailTokenResult(EmailTokenState::Invalid);
  }
}

std::string AuthService::createRedirectUrl(const std::string& token) const
{
  WApplication *app = WApplication::instance();
  return app->makeAbsoluteUrl(app->bookmarkUrl(redirectInternalPath_)) + token;
}

void AuthService::sendConfirmMail(const std::string& address,
				  const User& user, const std::string& token)
  const
{
  Mail::Message message;

  std::string url = createRedirectUrl(token);

  message.addRecipient(Mail::RecipientType::To, Mail::Mailbox(address));
  message.setSubject(WString::tr("Wt.Auth.confirmmail.subject"));
  message.setBody(WString::tr("Wt.Auth.confirmmail.body")
		  .arg(user.identity(Identity::LoginName))
		  .arg(token).arg(url));
  message.addHtmlBody(WString::tr("Wt.Auth.confirmmail.htmlbody")
		      .arg(user.identity(Identity::LoginName))
		      .arg(token).arg(url));

  sendMail(message);
}

void AuthService::sendLostPasswordMail(const std::string& address,
				    const User& user,
				    const std::string& token) const
{
  Mail::Message message;

  std::string url = createRedirectUrl(token);

  message.addRecipient(Mail::RecipientType::To, Mail::Mailbox(address));
  message.setSubject(WString::tr("Wt.Auth.lostpasswordmail.subject"));
  message.setBody(WString::tr("Wt.Auth.lostpasswordmail.body")
		  .arg(user.identity(Identity::LoginName))
		  .arg(token).arg(url));
  message.addHtmlBody(WString::tr("Wt.Auth.lostpasswordmail.htmlbody")
		      .arg(user.identity(Identity::LoginName))
		      .arg(token).arg(url));

  sendMail(message);
}

void AuthService::sendMail(const Mail::Message& message) const
{
  Mail::Message m = message;

  if (m.from().empty()) {
    std::string senderName = "Wt Auth module";
    std::string senderAddress = "noreply-auth@www.webtoolkit.eu";

    WApplication::readConfigurationProperty("auth-mail-sender-name",
					    senderName);
    WApplication::readConfigurationProperty("auth-mail-sender-address",
					    senderAddress);
#ifndef WT_TARGET_JAVA
    m.setFrom(Mail::Mailbox(senderAddress, WString::fromUTF8(senderName)));
#else
    m.setFrom(Mail::Mailbox(senderAddress, senderName));
#endif
  }

  m.write(std::cout);

  MailUtils::sendMail(m);
}

  }
}
