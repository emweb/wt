/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "AbstractUserDatabase.h"

#include "Wt/WDate.h"
#include "Wt/WDateTime.h"
#include "Wt/WException.h"
#include "Wt/WLogger.h"

namespace {
  const char *EMAIL_VERIFICATION = "email verification";
  const char *AUTH_TOKEN = "authentication tokens";
  const char *PASSWORDS = "password handling";
  const char *THROTTLING = "password attempt throttling";
  const char *REGISTRATION = "user registration";
  const char *IDP_SUPPORT = "identity provider support";
}

namespace Wt {

LOGGER("Auth.AbstractUserDatabase");

  namespace Auth {

class Require : public WException
{
public:
  Require(const std::string& method)
    : WException("You need to specialize " + method)
  { }

  Require(const std::string& method, const std::string& function)
    : WException("You need to specialize "
		 + method + " for " + function)
  { }
};

AbstractUserDatabase::Transaction::~Transaction() noexcept(false)
{ }

AbstractUserDatabase::AbstractUserDatabase()
{ }

AbstractUserDatabase::~AbstractUserDatabase()
{ }

AbstractUserDatabase::Transaction *AbstractUserDatabase::startTransaction()
{
  return nullptr;
}

AccountStatus AbstractUserDatabase::status(const User& user) const
{
  return AccountStatus::Normal;
}

void AbstractUserDatabase::setStatus(const User& user, AccountStatus status)
{
  LOG_ERROR(Require("setStatus()").what());
}

PasswordHash AbstractUserDatabase::password(const User& user) const
{
  LOG_ERROR(Require("password()", PASSWORDS).what());

  return PasswordHash();
}

void AbstractUserDatabase::setPassword(const User& user,
				       const PasswordHash& password)
{
  LOG_ERROR(Require("setPassword()", PASSWORDS).what());
}

void AbstractUserDatabase::setIdentity(const User& user,
				       const std::string& provider,
				       const WT_USTRING& id)
{
  removeIdentity(user, provider);
  addIdentity(user, provider, id);
}

User AbstractUserDatabase::registerNew()
{
  LOG_ERROR(Require("registerNew()", REGISTRATION).what());

  return User();
}

void AbstractUserDatabase::deleteUser(const User& user)
{
  LOG_ERROR(Require("deleteUser()", REGISTRATION).what());
}

std::string AbstractUserDatabase::email(const User& user) const
{
  LOG_ERROR(Require("email()", EMAIL_VERIFICATION).what());

  return std::string();
}

bool AbstractUserDatabase::setEmail(const User& user,
				    const std::string& address)
{
  LOG_ERROR(Require("setEmail()", EMAIL_VERIFICATION).what());
  return false;
}

User AbstractUserDatabase::findWithEmail(const std::string& address) const
{
  LOG_ERROR(Require("findWithEmail()", EMAIL_VERIFICATION).what());

  return User();
}

std::string AbstractUserDatabase::unverifiedEmail(const User& user) const
{
  LOG_ERROR(Require("unverifiedEmail()", EMAIL_VERIFICATION).what());

  return std::string();
}

void AbstractUserDatabase::setUnverifiedEmail(const User& user,
					      const std::string& address)
{
  LOG_ERROR(Require("setUnverifiedEmail()", EMAIL_VERIFICATION).what());
}

Token AbstractUserDatabase::emailToken(const User& user) const
{
  LOG_ERROR(Require("emailToken()", EMAIL_VERIFICATION).what());

  return Token();
}

EmailTokenRole AbstractUserDatabase::emailTokenRole(const User& user)
  const
{
  LOG_ERROR(Require("emailTokenRole()", EMAIL_VERIFICATION).what());

  return EmailTokenRole::VerifyEmail;
}

void AbstractUserDatabase::setEmailToken(const User& user, const Token& token,
					 EmailTokenRole role)
{
  LOG_ERROR(Require("setEmailToken()", EMAIL_VERIFICATION).what());
}

User AbstractUserDatabase::findWithEmailToken(const std::string& hash) const
{
  LOG_ERROR(Require("findWithEmailToken()", EMAIL_VERIFICATION).what());

  return User();
}

void AbstractUserDatabase::addAuthToken(const User& user, const Token& token)
{
  LOG_ERROR(Require("addAuthToken()", AUTH_TOKEN).what());
}

void AbstractUserDatabase::removeAuthToken(const User& user,
					   const std::string& hash)
{
  LOG_ERROR(Require("removeAuthToken()", AUTH_TOKEN).what());
}

int AbstractUserDatabase::updateAuthToken(const User& user,
					  const std::string& hash,
					  const std::string& newHash)
{
  LOG_WARN(Require("updateAuthToken()", AUTH_TOKEN).what());

  return -1;
}

User AbstractUserDatabase::findWithAuthToken(const std::string& hash) const
{
  LOG_ERROR(Require("findWithAuthToken()", AUTH_TOKEN).what());

  return User();
}

int AbstractUserDatabase::failedLoginAttempts(const User& user) const
{
  LOG_ERROR(Require("failedLoginAttempts()", THROTTLING).what());

  return 0;
}

void AbstractUserDatabase::setFailedLoginAttempts(const User& user, int count)
{ 
  LOG_ERROR(Require("setFailedLoginAttempts()", THROTTLING).what());
}

WDateTime AbstractUserDatabase::lastLoginAttempt(const User& user) const
{
  LOG_ERROR(Require("lastLoginAttempt()", THROTTLING).what());

  return WDateTime(WDate(1970, 1, 1));
}

void AbstractUserDatabase::setLastLoginAttempt(const User& user,
					       const WDateTime& t)
{
  LOG_ERROR(Require("setLastLoginAttempt()", THROTTLING).what());
}

// ...
Auth::IssuedToken AbstractUserDatabase::idpTokenAdd(const std::string& value,
                                                    const WDateTime& expirationTime,
                                                    const std::string& purpose,
                                                    const std::string& scope,
                                                    const std::string& redirectUri,
                                                    const User& user,
                                                    const OAuthClient& authClient)
{
	LOG_ERROR(Require("idpTokenAdd()", IDP_SUPPORT).what());
	return IssuedToken();
}

void AbstractUserDatabase::idpTokenRemove(const IssuedToken& token)
{
  LOG_ERROR(Require("idpTokenRemove()", IDP_SUPPORT).what());
}

IssuedToken AbstractUserDatabase::idpTokenFindWithValue(
  const std::string& purpose, const std::string& value) const
{
  LOG_ERROR(Require("idpTokenFindWithValue()", IDP_SUPPORT).what());
  return IssuedToken();
}

WDateTime AbstractUserDatabase::idpTokenExpirationTime(const IssuedToken &token) const
{
  LOG_ERROR(Require("idpTokenExpirationTime)", IDP_SUPPORT).what());
  return WDateTime(WDate(1970, 1, 1));
}

std::string AbstractUserDatabase::idpTokenValue(const IssuedToken &token) const
{
  LOG_ERROR(Require("idpTokenValue()", IDP_SUPPORT).what());
  return std::string();
}

std::string AbstractUserDatabase::idpTokenPurpose(const IssuedToken &token) const
{
  LOG_ERROR(Require("idpTokenPurpose()", IDP_SUPPORT).what());
  return std::string();
}

std::string AbstractUserDatabase::idpTokenScope(const IssuedToken &token) const
{
  LOG_ERROR(Require("idpTokenScope()", IDP_SUPPORT).what());
  return std::string();
}

std::string AbstractUserDatabase::idpTokenRedirectUri(const IssuedToken &token) const
{
  LOG_ERROR(Require("idpTokenRedirectUri()", IDP_SUPPORT).what());
  return std::string();
}

User AbstractUserDatabase::idpTokenUser(const IssuedToken &token) const
{
  LOG_ERROR(Require("idpTokenUser()", IDP_SUPPORT).what());
  return User();
}

OAuthClient AbstractUserDatabase::idpTokenOAuthClient(const IssuedToken &token) const
{
  LOG_ERROR(Require("idpTokenOAuthClient()", IDP_SUPPORT).what());
  return OAuthClient();
}

Json::Value AbstractUserDatabase::idpJsonClaim(const User& user, const std::string& claim) const
{
  LOG_ERROR(Require("idpClaim()", IDP_SUPPORT).what());
  return Wt::Json::Value::Null; // full namespace for cnor
}

OAuthClient AbstractUserDatabase::idpClientFindWithId(const std::string &clientId) const
{
  LOG_ERROR(Require("idpClientFindWithId()", IDP_SUPPORT).what());
  return OAuthClient();
}

std::string AbstractUserDatabase::idpClientSecret(const OAuthClient &client) const
{
  LOG_ERROR(Require("idpClientSecret()", IDP_SUPPORT).what());
  return std::string();
}

bool AbstractUserDatabase::idpVerifySecret(const OAuthClient &client, const std::string& secret) const
{
  LOG_ERROR(Require("idpVerifySecret()", IDP_SUPPORT).what());
  return false;
}

std::set<std::string> AbstractUserDatabase::idpClientRedirectUris(const OAuthClient &client) const
{
  LOG_ERROR(Require("idpClientRedirectUris()", IDP_SUPPORT).what());
  return std::set<std::string>();
}

std::string AbstractUserDatabase::idpClientId(const OAuthClient &client) const
{
  LOG_ERROR(Require("idpClientId()", IDP_SUPPORT).what());
  return std::string();
}

bool AbstractUserDatabase::idpClientConfidential(const OAuthClient &client) const
{
  LOG_ERROR(Require("idpClientConfidential()", IDP_SUPPORT).what());
  return false;
}

ClientSecretMethod AbstractUserDatabase::idpClientAuthMethod(const OAuthClient &client) const
{
  LOG_ERROR(Require("idpClientAuthMethod()", IDP_SUPPORT).what());
  return HttpAuthorizationBasic;
}

OAuthClient AbstractUserDatabase::idpClientAdd(const std::string& clientId,
    bool confidential,
    const std::set<std::string> &redirectUris,
    ClientSecretMethod authMethod,
    const std::string &secret)
{
  LOG_ERROR(Require("idpTokenAdd()", IDP_SUPPORT).what());
  return OAuthClient();
}

  }
}

