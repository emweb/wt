// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_SESSION_H_
#define BLOG_SESSION_H_

#include <Wt/WSignal>
#include <Wt/Auth/Login>
#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/backend/Sqlite3>

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

  BlogSession(const std::string& sqliteDb);

  dbo::ptr<User> user() const;

  Wt::Signal< dbo::ptr<Comment> >& commentsChanged()
    { return commentsChanged_; }

  BlogUserDatabase& users() { return users_; }
  Wt::Auth::Login& login() { return login_; }

  Wt::Auth::PasswordService *passwordAuth() const;
  const std::vector<const Wt::Auth::OAuthService *>& oAuth() const;

private:
  dbo::backend::Sqlite3 connection_;
  BlogUserDatabase users_;
  Wt::Auth::Login login_;

  Wt::Signal< dbo::ptr<Comment> > commentsChanged_;
};

#endif // BLOG_SESSION_H_
