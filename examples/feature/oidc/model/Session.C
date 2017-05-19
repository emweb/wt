#include "Session.h"

#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/PasswordStrengthValidator>

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

  Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction());
  myPasswordService.setVerifier(verifier);
  myPasswordService.setAttemptThrottlingEnabled(true);
//  myPasswordService.setStrengthValidator
//    (new Wt::Auth::PasswordStrengthValidator());
}

Session::Session(const std::string& db)
  : connection_(db)
{
  connection_.setProperty("show-queries", "true");

  setConnection(connection_);
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

  users_ = new OidcUserDatabase(*this);
}

Session::~Session()
{
  delete users_;
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
    user = add(new User());
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
