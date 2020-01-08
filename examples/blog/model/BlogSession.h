// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_SESSION_H_
#define BLOG_SESSION_H_

#include <Wt/WSignal.h>
#include <Wt/Auth/Login.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/backend/Sqlite3.h>

namespace Wt {
  namespace Auth {
    class OAuthService;
    class PasswordService;
  }
}

#include "BlogUserDatabase.h"

namespace dbo = Wt::Dbo;

class Comment;
class Post;
class User;

class BlogSession : public dbo::Session
{
public:
  static void configureAuth();

  BlogSession(dbo::SqlConnectionPool& connectionPool);

  dbo::ptr<User> user() const;

  Wt::Signal< dbo::ptr<Comment> >& commentsChanged()
    { return commentsChanged_; }

  BlogUserDatabase& users() { return users_; }
  Wt::Auth::Login& login() { return login_; }

  Wt::Auth::PasswordService *passwordAuth() const;
  const std::vector<const Wt::Auth::OAuthService *>& oAuth() const;

  static std::unique_ptr<dbo::SqlConnectionPool> createConnectionPool(const std::string& sqlite3);

private:
  dbo::SqlConnectionPool&     connectionPool_;
  BlogUserDatabase            users_;
  Wt::Auth::Login                 login_;

  Wt::Signal< dbo::ptr<Comment> > commentsChanged_;
};

#endif // BLOG_SESSION_H_
