/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"

#include "Wt/Auth/AuthService"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/PasswordStrengthValidator"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/GoogleService"
#include "Wt/Auth/FacebookService"
#include "Wt/Auth/Dbo/AuthInfo"
#include "Wt/Auth/Dbo/UserDatabase"

namespace {

  class MyOAuth : public std::vector<const Wt::Auth::OAuthService *>
  {
  public:
    ~MyOAuth()
    {
      for (unsigned i = 0; i < size(); ++i)
	delete (*this)[i];
    }
  };

  Wt::Auth::AuthService myAuthService;
  Wt::Auth::PasswordService myPasswordService(myAuthService);
  MyOAuth myOAuthServices;

}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  myAuthService.setEmailVerificationEnabled(true);
  myAuthService.setEmailVerificationRequired(true);

  Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
  myPasswordService.setVerifier(verifier);
  myPasswordService.setAttemptThrottlingEnabled(true);
  myPasswordService.setStrengthValidator
    (new Wt::Auth::PasswordStrengthValidator());

  if (Wt::Auth::GoogleService::configured())
    myOAuthServices.push_back(new Wt::Auth::GoogleService(myAuthService));

  if (Wt::Auth::FacebookService::configured())
    myOAuthServices.push_back(new Wt::Auth::FacebookService(myAuthService));

  for (unsigned i = 0; i < myOAuthServices.size(); ++i)
    myOAuthServices[i]->generateRedirectEndpoint();
}

Session::Session(const std::string& sqliteDb)
  : connection_(sqliteDb)
{
  connection_.setProperty("show-queries", "true");

  setConnection(connection_);

  mapClass<User>("user");
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

  users_ = new UserDatabase(*this);
}

Session::~Session()
{
  delete users_;
}

Wt::Auth::AbstractUserDatabase& Session::users()
{
  return *users_;
}

dbo::ptr<User> Session::user() const
{
  if (login_.loggedIn()) {
    dbo::ptr<AuthInfo> authInfo = users_->find(login_.user());
    return authInfo->user();
  } else
    return dbo::ptr<User>();
}

const Wt::Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Wt::Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}

const std::vector<const Wt::Auth::OAuthService *>& Session::oAuth()
{
  return myOAuthServices;
}
