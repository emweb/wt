// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <vector>

#include <Wt/Auth/Login.h>
#include <Wt/Auth/Dbo/UserDatabase.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "User.h"

using UserDatabase = Wt::Auth::Dbo::UserDatabase<AuthInfo>;

class Session
{
public:
  static void configureAuth();

  Session();

  Wt::Auth::AbstractUserDatabase& users();
  Wt::Auth::Login& login() { return login_; }

  std::vector<User> topUsers(int limit);

  /*
   * These methods deal with the currently logged in user
   */
  std::string userName() const;
  int findRanking();
  void addToScore(int s);

  static const Wt::Auth::AuthService& auth();
  static const Wt::Auth::AbstractPasswordService& passwordAuth();
  static std::vector<const Wt::Auth::OAuthService *> oAuth();

private:
  mutable Wt::Dbo::Session session_;
  std::unique_ptr<UserDatabase> users_;
  Wt::Auth::Login login_;

  Wt::Dbo::ptr<User> user() const;
};

#endif //SESSION_H_
