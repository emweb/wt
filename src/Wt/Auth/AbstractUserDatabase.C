/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WDate"
#include "Wt/WException"
#include "Wt/WLogger"

#include "AbstractUserDatabase"

namespace {
  const char *EMAIL_VERIFICATION = "email verification";
  const char *AUTH_TOKEN = "authentication tokens";
  const char *PASSWORDS = "password handling";
  const char *THROTTLING = "password attempt throttling";
  const char *REGISTRATION = "user registration";
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

  Require(const std::string& method, const std::string function)
    : WException("You need to specialize "
		 + method + " for " + function)
  { }
};

AbstractUserDatabase::Transaction::~Transaction() WT_CXX11ONLY(noexcept(false))
{ }

AbstractUserDatabase::AbstractUserDatabase()
{ }

AbstractUserDatabase::~AbstractUserDatabase()
{ }

AbstractUserDatabase::Transaction *AbstractUserDatabase::startTransaction()
{
  return 0;
}

User::Status AbstractUserDatabase::status(const User& user) const
{
  return User::Normal;
}

void AbstractUserDatabase::setStatus(const User& user, User::Status status)
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

User::EmailTokenRole AbstractUserDatabase::emailTokenRole(const User& user)
  const
{
  LOG_ERROR(Require("emailTokenRole()", EMAIL_VERIFICATION).what());

  return User::VerifyEmail;
}

void AbstractUserDatabase::setEmailToken(const User& user, const Token& token,
					 User::EmailTokenRole role)
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

  }
}
