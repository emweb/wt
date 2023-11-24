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

AccountStatus AbstractUserDatabase::status(WT_MAYBE_UNUSED const User& user) const
{
  return AccountStatus::Normal;
}

void AbstractUserDatabase::setStatus(WT_MAYBE_UNUSED const User& user, WT_MAYBE_UNUSED AccountStatus status)
{
  LOG_ERROR(Require("setStatus()").what());
}

PasswordHash AbstractUserDatabase::password(WT_MAYBE_UNUSED const User& user) const
{
  LOG_ERROR(Require("password()", PASSWORDS).what());

  return PasswordHash();
}

void AbstractUserDatabase::setPassword(WT_MAYBE_UNUSED const User& user,
                                       WT_MAYBE_UNUSED const PasswordHash& password)
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

void AbstractUserDatabase::deleteUser(WT_MAYBE_UNUSED const User& user)
{
  LOG_ERROR(Require("deleteUser()", REGISTRATION).what());
}

std::string AbstractUserDatabase::email(WT_MAYBE_UNUSED const User& user) const
{
  LOG_ERROR(Require("email()", EMAIL_VERIFICATION).what());

  return std::string();
}

bool AbstractUserDatabase::setEmail(WT_MAYBE_UNUSED const User& user,
                                    WT_MAYBE_UNUSED const std::string& address)
{
  LOG_ERROR(Require("setEmail()", EMAIL_VERIFICATION).what());
  return false;
}

User AbstractUserDatabase::findWithEmail(WT_MAYBE_UNUSED const std::string& address) const
{
  LOG_ERROR(Require("findWithEmail()", EMAIL_VERIFICATION).what());

  return User();
}

std::string AbstractUserDatabase::unverifiedEmail(WT_MAYBE_UNUSED const User& user) const
{
  LOG_ERROR(Require("unverifiedEmail()", EMAIL_VERIFICATION).what());

  return std::string();
}

void AbstractUserDatabase::setUnverifiedEmail(WT_MAYBE_UNUSED const User& user,
                                              WT_MAYBE_UNUSED const std::string& address)
{
  LOG_ERROR(Require("setUnverifiedEmail()", EMAIL_VERIFICATION).what());
}

Token AbstractUserDatabase::emailToken(WT_MAYBE_UNUSED const User& user) const
{
  LOG_ERROR(Require("emailToken()", EMAIL_VERIFICATION).what());

  return Token();
}

EmailTokenRole AbstractUserDatabase::emailTokenRole(WT_MAYBE_UNUSED const User& user)
  const
{
  LOG_ERROR(Require("emailTokenRole()", EMAIL_VERIFICATION).what());

  return EmailTokenRole::VerifyEmail;
}

void AbstractUserDatabase::setEmailToken(WT_MAYBE_UNUSED const User& user, WT_MAYBE_UNUSED const Token& token,
                                         WT_MAYBE_UNUSED EmailTokenRole role)
{
  LOG_ERROR(Require("setEmailToken()", EMAIL_VERIFICATION).what());
}

User AbstractUserDatabase::findWithEmailToken(WT_MAYBE_UNUSED const std::string& hash) const
{
  LOG_ERROR(Require("findWithEmailToken()", EMAIL_VERIFICATION).what());

  return User();
}

void AbstractUserDatabase::addAuthToken(WT_MAYBE_UNUSED const User& user, WT_MAYBE_UNUSED const Token& token)
{
  LOG_ERROR(Require("addAuthToken()", AUTH_TOKEN).what());
}

void AbstractUserDatabase::removeAuthToken(WT_MAYBE_UNUSED const User& user,
                                           WT_MAYBE_UNUSED const std::string& hash)
{
  LOG_ERROR(Require("removeAuthToken()", AUTH_TOKEN).what());
}

int AbstractUserDatabase::updateAuthToken(WT_MAYBE_UNUSED const User& user,
                                          WT_MAYBE_UNUSED const std::string& hash,
                                          WT_MAYBE_UNUSED const std::string& newHash)
{
  LOG_WARN(Require("updateAuthToken()", AUTH_TOKEN).what());

  return -1;
}

User AbstractUserDatabase::findWithAuthToken(WT_MAYBE_UNUSED const std::string& hash) const
{
  LOG_ERROR(Require("findWithAuthToken()", AUTH_TOKEN).what());

  return User();
}

int AbstractUserDatabase::failedLoginAttempts(WT_MAYBE_UNUSED const User& user) const
{
  LOG_ERROR(Require("failedLoginAttempts()", THROTTLING).what());

  return 0;
}

void AbstractUserDatabase::setFailedLoginAttempts(WT_MAYBE_UNUSED const User& user, WT_MAYBE_UNUSED int count)
{
  LOG_ERROR(Require("setFailedLoginAttempts()", THROTTLING).what());
}

WDateTime AbstractUserDatabase::lastLoginAttempt(WT_MAYBE_UNUSED const User& user) const
{
  LOG_ERROR(Require("lastLoginAttempt()", THROTTLING).what());

  return WDateTime(WDate(1970, 1, 1));
}

void AbstractUserDatabase::setLastLoginAttempt(WT_MAYBE_UNUSED const User& user,
                                               WT_MAYBE_UNUSED const WDateTime& timestamp)
{
  LOG_ERROR(Require("setLastLoginAttempt()", THROTTLING).what());
}

// ...
Auth::IssuedToken AbstractUserDatabase::idpTokenAdd(WT_MAYBE_UNUSED const std::string& value,
                                                    WT_MAYBE_UNUSED const WDateTime& expirationTime,
                                                    WT_MAYBE_UNUSED const std::string& purpose,
                                                    WT_MAYBE_UNUSED const std::string& scope,
                                                    WT_MAYBE_UNUSED const std::string& redirectUri,
                                                    WT_MAYBE_UNUSED const User& user,
                                                    WT_MAYBE_UNUSED const OAuthClient& authClient)
{
  LOG_ERROR(Require("idpTokenAdd()", IDP_SUPPORT).what());
  return IssuedToken();
}

void AbstractUserDatabase::idpTokenRemove(WT_MAYBE_UNUSED const IssuedToken& token)
{
  LOG_ERROR(Require("idpTokenRemove()", IDP_SUPPORT).what());
}

IssuedToken AbstractUserDatabase::idpTokenFindWithValue(
  WT_MAYBE_UNUSED const std::string& purpose, WT_MAYBE_UNUSED const std::string& scope) const
{
  LOG_ERROR(Require("idpTokenFindWithValue()", IDP_SUPPORT).what());
  return IssuedToken();
}

WDateTime AbstractUserDatabase::idpTokenExpirationTime(WT_MAYBE_UNUSED const IssuedToken& token) const
{
  LOG_ERROR(Require("idpTokenExpirationTime)", IDP_SUPPORT).what());
  return WDateTime(WDate(1970, 1, 1));
}

std::string AbstractUserDatabase::idpTokenValue(WT_MAYBE_UNUSED const IssuedToken& token) const
{
  LOG_ERROR(Require("idpTokenValue()", IDP_SUPPORT).what());
  return std::string();
}

std::string AbstractUserDatabase::idpTokenPurpose(WT_MAYBE_UNUSED const IssuedToken& token) const
{
  LOG_ERROR(Require("idpTokenPurpose()", IDP_SUPPORT).what());
  return std::string();
}

std::string AbstractUserDatabase::idpTokenScope(WT_MAYBE_UNUSED const IssuedToken& token) const
{
  LOG_ERROR(Require("idpTokenScope()", IDP_SUPPORT).what());
  return std::string();
}

std::string AbstractUserDatabase::idpTokenRedirectUri(WT_MAYBE_UNUSED const IssuedToken& token) const
{
  LOG_ERROR(Require("idpTokenRedirectUri()", IDP_SUPPORT).what());
  return std::string();
}

User AbstractUserDatabase::idpTokenUser(WT_MAYBE_UNUSED const IssuedToken& token) const
{
  LOG_ERROR(Require("idpTokenUser()", IDP_SUPPORT).what());
  return User();
}

OAuthClient AbstractUserDatabase::idpTokenOAuthClient(WT_MAYBE_UNUSED const IssuedToken& token) const
{
  LOG_ERROR(Require("idpTokenOAuthClient()", IDP_SUPPORT).what());
  return OAuthClient();
}

Json::Value AbstractUserDatabase::idpJsonClaim(WT_MAYBE_UNUSED const User& user, WT_MAYBE_UNUSED const std::string& claim) const
{
  LOG_ERROR(Require("idpClaim()", IDP_SUPPORT).what());
  return Wt::Json::Value::Null; // full namespace for cnor
}

OAuthClient AbstractUserDatabase::idpClientFindWithId(WT_MAYBE_UNUSED const std::string& clientId) const
{
  LOG_ERROR(Require("idpClientFindWithId()", IDP_SUPPORT).what());
  return OAuthClient();
}

std::string AbstractUserDatabase::idpClientSecret(WT_MAYBE_UNUSED const OAuthClient& client) const
{
  LOG_ERROR(Require("idpClientSecret()", IDP_SUPPORT).what());
  return std::string();
}

bool AbstractUserDatabase::idpVerifySecret(WT_MAYBE_UNUSED const OAuthClient& client, WT_MAYBE_UNUSED const std::string& secret) const
{
  LOG_ERROR(Require("idpVerifySecret()", IDP_SUPPORT).what());
  return false;
}

std::set<std::string> AbstractUserDatabase::idpClientRedirectUris(WT_MAYBE_UNUSED const OAuthClient& client) const
{
  LOG_ERROR(Require("idpClientRedirectUris()", IDP_SUPPORT).what());
  return std::set<std::string>();
}

std::string AbstractUserDatabase::idpClientId(WT_MAYBE_UNUSED const OAuthClient& client) const
{
  LOG_ERROR(Require("idpClientId()", IDP_SUPPORT).what());
  return std::string();
}

bool AbstractUserDatabase::idpClientConfidential(WT_MAYBE_UNUSED const OAuthClient& client) const
{
  LOG_ERROR(Require("idpClientConfidential()", IDP_SUPPORT).what());
  return false;
}

ClientSecretMethod AbstractUserDatabase::idpClientAuthMethod(WT_MAYBE_UNUSED const OAuthClient& client) const
{
  LOG_ERROR(Require("idpClientAuthMethod()", IDP_SUPPORT).what());
  return HttpAuthorizationBasic;
}

OAuthClient AbstractUserDatabase::idpClientAdd(WT_MAYBE_UNUSED const std::string& clientId,
    WT_MAYBE_UNUSED bool confidential,
    WT_MAYBE_UNUSED const std::set<std::string>& redirectUris,
    WT_MAYBE_UNUSED ClientSecretMethod authMethod,
    WT_MAYBE_UNUSED const std::string& secret)
{
  LOG_ERROR(Require("idpTokenAdd()", IDP_SUPPORT).what());
  return OAuthClient();
}

  }
}

