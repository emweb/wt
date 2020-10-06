#include "Session.h"

#include <Wt/Auth/Dbo/AuthInfo.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordStrengthValidator.h>

#include "User.h"
#include "IssuedToken.h"
#include "OAuthClient.h"
#include "OidcUserDatabase.h"

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

namespace {
Wt::Auth::AuthService myAuthService;
Wt::Auth::PasswordService myPasswordService(myAuthService);
}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  myAuthService.setEmailVerificationEnabled(true);

  auto verifier = std::make_unique<Wt::Auth::PasswordVerifier>();
  verifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>());
  myPasswordService.setVerifier(std::move(verifier));
  myPasswordService.setAttemptThrottlingEnabled(true);
//  myPasswordService.setStrengthValidator
//    (std::make_unique<Wt::Auth::PasswordStrengthValidator>());
}

Session::Session(const std::string& db)
{
  auto connection = std::make_unique<Wt::Dbo::backend::Sqlite3>(db);

  connection->setProperty("show-queries", "true");

  setConnection(std::move(connection));
  mapClass<User>("user");
  mapClass<IssuedToken>("issued_token");
  mapClass<OAuthClient>("oauth_client");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");

  try {
    createTables();
    std::cerr << "Created database." << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
  }

  users_ = std::make_unique<OidcUserDatabase>(*this);
}

Wt::Auth::AbstractUserDatabase& Session::users()
{
  return *users_;
}

Wt::Dbo::ptr<User> Session::user()
{
  if (login_.loggedIn())
    return user(login_.user());
  else
    return Wt::Dbo::ptr<User>();
}

Wt::Dbo::ptr<User> Session::user(const Wt::Auth::User& authUser)
{
  Wt::Dbo::ptr<AuthInfo> authInfo = users_->find(authUser);
  Wt::Dbo::ptr<User> user = authInfo->user();

  if (!user) {
    user = addNew<User>();
    authInfo.modify()->setUser(user);
  }
  return user;
}

const Wt::Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Wt::Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}
