/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WDate"
#include "Wt/WException"
#include "Wt/WLogger"

#include "AbstractUserDatabase"

namespace Wt {
  namespace Auth {

const char *EMAIL_VERIFICATION = "email verification";
const char *AUTH_TOKEN = "authentication tokens";
const char *PASSWORDS = "password handling";
const char *THROTTLING = "password attempt throttling";
const char *REGISTRATION = "user registration";
const char *OAUTH = "OAuth handling";

class Require : public WException
{
public:
  Require(const std::string& method)
    : WException("You need to specialize AbstractUserDatabase::" + method)
  { }

  Require(const std::string& method, const std::string function)
    : WException("You need to specialize AbstractUserDatabase::"
		 + method + " for " + function)
  { }
};

AbstractUserDatabase::Transaction::~Transaction()
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

PasswordHash AbstractUserDatabase::password(const User& user) const
{
  Wt::log("error") << Require("password()", PASSWORDS).what();

  return PasswordHash();
}

void AbstractUserDatabase::setPassword(const User& user,
				       const PasswordHash& password)
{
  Wt::log("error") << Require("setPassword()", PASSWORDS).what();
}

void AbstractUserDatabase::addOAuthId(const User& user, const std::string& id)
{
  Wt::log("error") << Require("addOAuthId()", OAUTH).what();
}

User AbstractUserDatabase::findOAuthId(const std::string& identity) const
{
  Wt::log("error") << Require("findOAuthId()", OAUTH).what();

  return User();
}

User AbstractUserDatabase::registerNew(const WT_USTRING& identity)
{
  Wt::log("error") << Require("registerNew()", REGISTRATION).what();

  return User();
}

std::string AbstractUserDatabase::email(const User& user) const
{
  Wt::log("error") << Require("email()", EMAIL_VERIFICATION).what();

  return std::string();
}

void AbstractUserDatabase::setEmail(const User& user,
				    const std::string& address)
{
  Wt::log("error") << Require("setEmail()", EMAIL_VERIFICATION).what();
}

User AbstractUserDatabase::findWithEmail(const std::string& address) const
{
  Wt::log("error") << Require("findWithEmail()", EMAIL_VERIFICATION).what();

  return User();
}

std::string AbstractUserDatabase::unverifiedEmail(const User& user) const
{
  Wt::log("error") << Require("unverifiedEmail()", EMAIL_VERIFICATION).what();

  return std::string();
}

void AbstractUserDatabase::setUnverifiedEmail(const User& user,
					      const std::string& address)
{
  Wt::log("error")<< Require("setUnverifiedEmail()", EMAIL_VERIFICATION).what();
}

Token AbstractUserDatabase::emailToken(const User& user) const
{
  Wt::log("error") << Require("emailToken()", EMAIL_VERIFICATION).what();

  return Token();
}

User::EmailTokenRole AbstractUserDatabase::emailTokenRole(const User& user)
  const
{
  Wt::log("error") << Require("emailTokenRole()", EMAIL_VERIFICATION).what();

  return User::VerifyEmail;
}

void AbstractUserDatabase::setEmailToken(const User& user, const Token& token,
					 User::EmailTokenRole role)
{
  Wt::log("error") << Require("setEmailToken()", EMAIL_VERIFICATION).what();
}

User AbstractUserDatabase::findWithEmailToken(const std::string& hash) const
{
  Wt::log("error")
    << Require("findWithEmailToken()", EMAIL_VERIFICATION).what();

  return User();
}

void AbstractUserDatabase::addAuthToken(const User& user, const Token& token)
{
  Wt::log("error") << Require("addAuthToken()", AUTH_TOKEN).what();
}

void AbstractUserDatabase::removeAuthToken(const User& user,
					   const std::string& hash)
{
  Wt::log("error") << Require("removeAuthToken()", AUTH_TOKEN).what();
}

User AbstractUserDatabase::findWithAuthToken(const std::string& hash) const
{
  Wt::log("error") << Require("findWithAuthToken()", AUTH_TOKEN).what();

  return User();
}

int AbstractUserDatabase::failedLoginAttempts(const User& user) const
{
  Wt::log("error") << Require("failedLoginAttempts()", THROTTLING).what();

  return 0;
}

void AbstractUserDatabase::setFailedLoginAttempts(const User& user, int count)
{ 
  Wt::log("error") << Require("setFailedLoginAttempts()", THROTTLING).what();
}

WDateTime AbstractUserDatabase::lastLoginAttempt(const User& user) const
{
  Wt::log("error") << Require("lastLoginAttempt()", THROTTLING).what();

  return WDateTime(WDate(1970, 1, 1));
}

void AbstractUserDatabase::setLastLoginAttempt(const User& user,
					       const WDateTime& t)
{ 
  Wt::log("error") << Require("setLastLoginAttempt()", THROTTLING).what();
}

  }
}
