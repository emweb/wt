// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SESSION_H_
#define SESSION_H_

#include <Wt/Auth/Login.h>
#include <Wt/Auth/Dbo/UserDatabase.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>

#include "User.h"

namespace dbo = Wt::Dbo;

using UserDatabase = Wt::Auth::Dbo::UserDatabase<AuthInfo>;

class Session : public dbo::Session
{
public:
  static void configureAuth();

  Session(const std::string& sqliteDb);

  dbo::ptr<User> user() const;

  Wt::Auth::AbstractUserDatabase& users();
  Wt::Auth::Login& login() { return login_; }

  static const Wt::Auth::AuthService& auth();
  static const Wt::Auth::PasswordService& passwordAuth();
  static const std::vector<const Wt::Auth::OAuthService *> oAuth();

private:
  std::unique_ptr<UserDatabase> users_;
  Wt::Auth::Login login_;
};

#endif // SESSION_H_
