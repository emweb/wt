// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
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

using namespace Wt;

namespace dbo = Dbo;

typedef Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class Session : public dbo::Session
{
public:
  static void configureAuth();

  Session(const std::string& sqliteDb);

  dbo::ptr<User> user() const;

  Auth::AbstractUserDatabase& users();
  Auth::Login& login() { return login_; }

  static const Auth::AuthService& auth();

private:
  std::unique_ptr<UserDatabase> users_;
  Auth::Login login_;
};

#endif // SESSION_H_
