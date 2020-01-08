// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_USER_DATABASE_H_
#define BLOG_USER_DATABASE_H_

#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Dbo/Types.h>

class User;

namespace dbo = Wt::Dbo;

class BlogUserDatabase : public Wt::Auth::AbstractUserDatabase
{
public:
  BlogUserDatabase(dbo::Session& session);

  virtual Transaction *startTransaction() override;

  dbo::ptr<User> find(const Wt::Auth::User& user) const;
  Wt::Auth::User find(dbo::ptr<User> user) const;

  virtual Wt::Auth::User findWithId(const std::string& id) const override;

  virtual Wt::Auth::User findWithIdentity(const std::string& provider,
                                          const Wt::WString& identity) const override;

  virtual void addIdentity(const Wt::Auth::User& user,
			   const std::string& provider,
                           const Wt::WString& identity) override;

  virtual Wt::WString identity(const Wt::Auth::User& user,
                               const std::string& provider) const override;

  virtual void removeIdentity(const Wt::Auth::User& user,
                              const std::string& provider) override;

  virtual Wt::Auth::PasswordHash password(const Wt::Auth::User& user) const override;
  virtual void setPassword(const Wt::Auth::User& user,
                           const Wt::Auth::PasswordHash& password) override;

  virtual Wt::Auth::User::Status status(const Wt::Auth::User& user) const override;
  virtual void setStatus(const Wt::Auth::User& user,
                         Wt::Auth::User::Status status) override;

  virtual Wt::Auth::User registerNew() override;

  virtual void addAuthToken(const Wt::Auth::User& user,
                            const Wt::Auth::Token& token) override;
  virtual int updateAuthToken(const Wt::Auth::User& user,
			      const std::string& hash,
			      const std::string& newHash) override;
  virtual void removeAuthToken(const Wt::Auth::User& user,
                               const std::string& hash) override;
  virtual Wt::Auth::User findWithAuthToken(const std::string& hash) const override;

  virtual int failedLoginAttempts(const Wt::Auth::User& user) const override;
  virtual void setFailedLoginAttempts(const Wt::Auth::User& user, int count) override;

  virtual Wt::WDateTime lastLoginAttempt(const Wt::Auth::User& user)
    const override;
  virtual void setLastLoginAttempt(const Wt::Auth::User& user,
                                   const Wt::WDateTime& t) override;

private:
  dbo::Session& session_;
  mutable dbo::ptr<User> user_;

  struct WithUser {
    WithUser(const BlogUserDatabase& self, const Wt::Auth::User& user);
    ~WithUser();

    dbo::Transaction transaction;
  };

  void getUser(const std::string& id) const;
};

#endif // USER_H_
