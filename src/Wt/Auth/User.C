/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "User.h"
#include "AbstractUserDatabase.h"
#include "Wt/WException.h"

namespace Wt {
  namespace Auth {

User::User()
  : db_(nullptr)
{ }

User::User(const std::string& id, const AbstractUserDatabase& userDatabase)
  : id_(id),
    db_(const_cast<AbstractUserDatabase *>(&userDatabase))
{ }

bool User::operator==(const User& other) const
{
  return id_ == other.id_ && db_ == other.db_;
}

bool User::operator!=(const User& other) const
{
  return !(*this == other);
}

PasswordHash User::password() const
{
  checkValid();

  return db_->password(*this);
}

void User::setPassword(const PasswordHash& password) const
{
  checkValid();

  db_->setPassword(*this, password);

  clearEmailToken();
}

std::string User::email() const
{
  return db_->email(*this);
}

void User::setEmail(const std::string& address) const
{
  checkValid();

  db_->setEmail(*this, address);
}

std::string User::unverifiedEmail() const
{
  checkValid();

  return db_->unverifiedEmail(*this);
}

void User::setUnverifiedEmail(const std::string& address) const
{
  checkValid();

  db_->setUnverifiedEmail(*this, address);
}

AccountStatus User::status() const
{
  checkValid();

  return db_->status(*this);
}

void User::setStatus(AccountStatus status)
{
  checkValid();

  db_->setStatus(*this, status);
}

void User::addIdentity(const std::string& provider, const WT_USTRING& identity)
{
  checkValid();

  db_->addIdentity(*this, provider, identity);
}

void User::setIdentity(const std::string& provider, const WT_USTRING& identity)
{
  checkValid();

  db_->setIdentity(*this, provider, identity);
}

void User::removeIdentity(const std::string& provider)
{
  checkValid();

  db_->removeIdentity(*this, provider);
}

WT_USTRING User::identity(const std::string& provider) const
{
  checkValid();

  return db_->identity(*this, provider);
}

Token User::emailToken() const
{
  return db_->emailToken(*this);
}

EmailTokenRole User::emailTokenRole() const
{
  return db_->emailTokenRole(*this);
}

void User::setEmailToken(const Token& token, EmailTokenRole role) const
{
  checkValid();

  db_->setEmailToken(*this, token, role);
}

void User::clearEmailToken() const
{
  checkValid();

  db_->setEmailToken(*this, Token(), EmailTokenRole::LostPassword);
}

void User::addAuthToken(const Token& token) const
{
  checkValid();

  db_->addAuthToken(*this, token);
}

void User::removeAuthToken(const std::string& token) const
{
  checkValid();

  db_->removeAuthToken(*this, token);
}

int User::updateAuthToken(const std::string& hash,
			  const std::string& newHash) const
{
  checkValid();

  return db_->updateAuthToken(*this, hash, newHash);
}

int User::failedLoginAttempts() const
{
  return db_->failedLoginAttempts(*this);
}

WDateTime User::lastLoginAttempt() const
{
  return db_->lastLoginAttempt(*this);
}

void User::setAuthenticated(bool success) const
{
  checkValid();

  if (success)
    db_->setFailedLoginAttempts(*this, 0);
  else
    db_->setFailedLoginAttempts(*this, db_->failedLoginAttempts(*this) + 1);

  db_->setLastLoginAttempt(*this, WDateTime::currentDateTime());
}

void User::checkValid() const
{
  if (!db_)
    throw WException("Method called on invalid Auth::User");
}


  }
}
