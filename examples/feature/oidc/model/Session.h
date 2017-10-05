#ifndef SESSION_H
#define SESSION_H

#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/User.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "User.h"

typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class Session : public Wt::Dbo::Session
{
public:
  static void configureAuth();

  Session(const std::string& db);

  Wt::Dbo::ptr<User> user();
  Wt::Dbo::ptr<User> user(const Wt::Auth::User& user);

  Wt::Auth::AbstractUserDatabase& users();
  Wt::Auth::Login& login() { return login_; }

  static const Wt::Auth::AuthService& auth();
  static const Wt::Auth::PasswordService& passwordAuth();

private:
  std::unique_ptr<UserDatabase> users_;
  Wt::Auth::Login login_;
};

#endif // SESSION_H
