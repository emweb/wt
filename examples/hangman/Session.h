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

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "User.h"

using namespace Wt;

typedef Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class Session
{
public:
  static void configureAuth();

  Session();
  ~Session();

  Auth::AbstractUserDatabase& users();
  Auth::Login& login() { return login_; }

  std::vector<User> topUsers(int limit);

  /*
   * These methods deal with the currently logged in user
   */
  std::string userName() const;
  int findRanking();
  void addToScore(int s);

  static const Auth::AuthService& auth();
  static const Auth::AbstractPasswordService& passwordAuth();
  static const std::vector<const Auth::OAuthService *>& oAuth();

private:
  mutable Dbo::Session session_;
  std::unique_ptr<UserDatabase> users_;
  Auth::Login login_;

  Dbo::ptr<User> user() const;
};

#endif //SESSION_H_
