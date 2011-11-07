// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_USER_DATABASE_H_
#define BLOG_USER_DATABASE_H_

#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/Dbo/Types>

class User;

namespace dbo = Wt::Dbo;

class BlogUserDatabase : public Wt::Auth::AbstractUserDatabase
{
public:
  BlogUserDatabase(dbo::Session& session);

  Transaction *startTransaction();

  dbo::ptr<User> find(const Wt::Auth::User& user) const;
  Wt::Auth::User find(dbo::ptr<User> user) const;

  virtual Wt::Auth::User findIdentity(const Wt::WString& userName) const;

  virtual Wt::Auth::PasswordHash password(const Wt::Auth::User& user) const;
  virtual void setPassword(const Wt::Auth::User& user,
			   const Wt::Auth::PasswordHash& password);

  virtual Wt::Auth::User::Status status(const Wt::Auth::User& user) const;
  virtual void setStatus(const Wt::Auth::User& user,
			 Wt::Auth::User::Status status);

  virtual void addOAuthId(const Wt::Auth::User& user, const std::string& id);
  virtual Wt::Auth::User findOAuthId(const std::string& identity) const;

  virtual Wt::Auth::User registerNew(const Wt::WString& identity);

  virtual void addAuthToken(const Wt::Auth::User& user,
			    const Wt::Auth::Token& token);
  virtual void removeAuthToken(const Wt::Auth::User& user,
			       const std::string& hash);
  virtual Wt::Auth::User findWithAuthToken(const std::string& hash) const;

  virtual int failedLoginAttempts(const Wt::Auth::User& user) const;
  virtual void setFailedLoginAttempts(const Wt::Auth::User& user, int count);

  virtual Wt::WDateTime lastLoginAttempt(const Wt::Auth::User& user)
    const;
  virtual void setLastLoginAttempt(const Wt::Auth::User& user,
				   const Wt::WDateTime& t);

private:
  dbo::Session& session_;
  mutable dbo::ptr<User> user_;

  struct WithUser {
    WithUser(const BlogUserDatabase& self, const Wt::Auth::User& user);
    ~WithUser();

    dbo::Transaction transaction;
  };

  void getUser(const Wt::WString& userName) const;
};

#endif // USER_H_
