#pragma once

#include "myuser.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/Dbo/UserDatabase.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>

using UserDatabase = Wt::Auth::Dbo::UserDatabase<AuthInfo>;

class MySession : public Wt::Dbo::Session
{
public:
  explicit MySession(const std::string& sqliteDb);

  void configureAuth();

  Wt::Auth::AbstractUserDatabase& users();
  Wt::Auth::Login& login() { return login_; }

  const Wt::Auth::AuthService& auth();
  const Wt::Auth::PasswordService& passwordAuth();

private:
  std::unique_ptr<UserDatabase> users_;
  Wt::Auth::Login login_;

  bool created_ = false;
};
