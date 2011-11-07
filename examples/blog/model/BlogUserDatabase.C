/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/Dbo/Impl>

#include "BlogUserDatabase.h"
#include "User.h"
#include "Token.h"

using namespace Wt;

class InvalidUser : public std::runtime_error
{
public:
  InvalidUser(const WT_USTRING& identity)
    : std::runtime_error("Invalid user: " + identity.toUTF8())
  { }
};

class TransactionImpl : public Auth::AbstractUserDatabase::Transaction,
			public dbo::Transaction
{
public:
  TransactionImpl(dbo::Session& session)
    : dbo::Transaction(session)
  { }

  virtual ~TransactionImpl()
  { }

  virtual void commit()
  {
    dbo::Transaction::commit();
  }

  virtual void rollback()
  {
    dbo::Transaction::rollback();
  }
};

BlogUserDatabase::BlogUserDatabase(dbo::Session& session)
  : session_(session)
{ }

Auth::AbstractUserDatabase::Transaction *BlogUserDatabase::startTransaction()
{
  return new TransactionImpl(session_);
}

dbo::ptr<User> BlogUserDatabase::find(const Auth::User& user) const
{
  getUser(user.identity());

  return user_;
}

Auth::User BlogUserDatabase::find(const dbo::ptr<User> user) const
{
  user_ = user;

  return Auth::User(user_->name, *this);
}

Auth::User BlogUserDatabase::findIdentity(const WString& userName) const
{
  getUser(userName);

  if (user_)
    return Auth::User(userName, *this);
  else
    return Auth::User();
}

Auth::PasswordHash BlogUserDatabase::password(const Auth::User& user) const
{
  WithUser find(*this, user);

  std::string salt = user_->passwordSalt;
  if (salt.empty() && user_->password.length() > 3)
    salt = user_->password.substr(3);

  return Auth::PasswordHash(user_->passwordMethod, salt, user_->password);
}

void BlogUserDatabase::setPassword(const Auth::User& user,
				   const Auth::PasswordHash& password)
{
  WithUser find(*this, user);

  user_.modify()->password = password.value();
  user_.modify()->passwordMethod = password.function();
  user_.modify()->passwordSalt = password.salt();
}

Auth::User::Status BlogUserDatabase::status(const Auth::User& user) const
{
  return Auth::User::Normal;
}

void BlogUserDatabase::setStatus(const Auth::User& user,
				 Auth::User::Status status)
{
  throw std::runtime_error("Changing status is not supported.");
}

void BlogUserDatabase::addOAuthId(const Auth::User& user, const std::string& id)
{
  WithUser find(*this, user);

  user_.modify()->oAuthId = id;
}

Auth::User BlogUserDatabase::findOAuthId(const std::string& id) const
{
  dbo::Transaction t(session_);
  user_ = session_.find<User>().where("oauth_id = ?").bind(id);
  t.commit();

  if (user_)
    return Auth::User(user_->name, *this);
  else
    return Auth::User();
}

Auth::User BlogUserDatabase::registerNew(const Wt::WString& identity)
{
  dbo::ptr<User> user = session_.find<User>().where("name = ?").bind(identity);
  if (user)
    return Auth::User();
  else {
    User *user = new User();
    user->name = identity;
    user_ = session_.add(user);
    return Auth::User(identity, *this);
  }
}

void BlogUserDatabase::addAuthToken(const Auth::User& user,
				    const Auth::Token& token)
{
  WithUser find(*this, user);

  /*
   * This should be statistically very unlikely but also a big
   * security problem if we do not detect it ...
   */
  if (session_.find<Token>().where("value = ?")
      .bind(token.hash()).resultList().size() > 0)
    throw std::runtime_error("Token hash collision");

  /*
   * Prevent a user from piling up the database with tokens
   */
  if (user_->authTokens.size() > 50)
    return;

  user_.modify()->authTokens.insert
    (dbo::ptr<Token>(new Token(token.hash(), token.expirationTime())));
}

void BlogUserDatabase::removeAuthToken(const Auth::User& user,
				       const std::string& hash)
{
  WithUser find(*this, user);

  for (Tokens::const_iterator i = user_->authTokens.begin();
       i != user_->authTokens.end(); ++i) {
    if ((*i)->value == hash) {
      dbo::ptr<Token> p = *i;
      p.remove();
      break;
    }
  }
}

Auth::User BlogUserDatabase::findWithAuthToken(const std::string& hash) const
{
  dbo::Transaction t(session_);
  user_ = session_.query< dbo::ptr<User> >
    ("select u from user u join token t on u.id = t.user_id")
    .where("t.value = ?").bind(hash)
    .where("t.expires > ?").bind(WDateTime::currentDateTime());
  t.commit();

  if (user_)
    return Auth::User(user_->name, *this);
  else
    return Auth::User();
}

int BlogUserDatabase::failedLoginAttempts(const Auth::User& user) const
{
  WithUser find(*this, user);

  return user_->failedLoginAttempts;
}

void BlogUserDatabase::setFailedLoginAttempts(const Auth::User& user,
					      int count)
{
  WithUser find(*this, user);

  user_.modify()->failedLoginAttempts = count;
}

WDateTime BlogUserDatabase::lastLoginAttempt(const Auth::User& user) const
{
  WithUser find(*this, user);

  return user_->lastLoginAttempt;
}

void BlogUserDatabase::setLastLoginAttempt(const Auth::User& user,
					   const WDateTime& t)
{ 
  WithUser find(*this, user);

  user_.modify()->lastLoginAttempt = t;
}

void BlogUserDatabase::getUser(const WString& userName) const
{
  if (!user_ || user_->name != userName) {
    dbo::Transaction t(session_);
    user_ = session_.find<User>().where("name = ?").bind(userName);
    t.commit();
  }
}

BlogUserDatabase::WithUser::WithUser(const BlogUserDatabase& self,
				     const Auth::User& user)
  : transaction(self.session_)
{
  self.getUser(user.identity());

  if (!self.user_)
    throw InvalidUser(user.identity());
}

BlogUserDatabase::WithUser::~WithUser()
{
  transaction.commit();
}
