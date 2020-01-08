// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SESSION_H_
#define SESSION_H_

#include <Wt/Auth/Login.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>

#include "User.h"

using namespace Wt;

namespace dbo = Dbo;

typedef Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class QRAuthService;
class QRTokenDatabase;

class Session : public dbo::Session
{
public:
  static void configureAuth();

  Session(const std::string& sqliteDb);
  ~Session();

  dbo::ptr<User> user() const;

  Auth::AbstractUserDatabase& users();
  QRTokenDatabase& qrTokenDatabase();
  Auth::Login& login() { return login_; }

  static const Auth::AuthService& auth();
  static const Auth::PasswordService& passwordAuth();
  static const QRAuthService& qrAuth();
  static const std::vector<const Auth::OAuthService *>& oAuth();

private:
  std::unique_ptr<UserDatabase> users_;
  std::unique_ptr<QRTokenDatabase> qrTokens_;
  Auth::Login login_;
};

#endif // SESSION_H_
