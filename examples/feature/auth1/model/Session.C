/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/GoogleService.h>
#include <Wt/Auth/FacebookService.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

#include <Wt/Dbo/backend/Sqlite3.h>

using namespace Wt;

namespace {

  Auth::AuthService myAuthService;
  Auth::PasswordService myPasswordService(myAuthService);
  std::vector<std::unique_ptr<Auth::OAuthService>> myOAuthServices;

}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  myAuthService.setEmailVerificationEnabled(true);
  myAuthService.setEmailVerificationRequired(true);

  auto verifier = std::make_unique<Auth::PasswordVerifier>();
  verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(12));
  myPasswordService.setVerifier(std::move(verifier));
  myPasswordService.setPasswordThrottle(std::make_unique<Wt::Auth::AuthThrottle>());
  myPasswordService.setStrengthValidator(
    std::make_unique<Auth::PasswordStrengthValidator>());

  if (Auth::GoogleService::configured()) {
    myOAuthServices.push_back(std::make_unique<Auth::GoogleService>(myAuthService));
  }

  if (Auth::FacebookService::configured()) {
    myOAuthServices.push_back(std::make_unique<Auth::FacebookService>(myAuthService));
  }

  for (const auto& oAuthService : myOAuthServices) {
    oAuthService->generateRedirectEndpoint();
  }
}

Session::Session(const std::string& sqliteDb)
{
  auto connection = std::make_unique<Dbo::backend::Sqlite3>(sqliteDb);

  connection->setProperty("show-queries", "true");

  setConnection(std::move(connection));

  mapClass<User>("user");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");

  try {
    createTables();
    std::cerr << "Created database.\n";
  } catch (Wt::Dbo::Exception& e) {
    std::cerr << e.what() << '\n';
    std::cerr << "Using existing database\n";
  }

  users_ = std::make_unique<UserDatabase>(*this);
}

Auth::AbstractUserDatabase& Session::users()
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

const Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}

std::vector<const Auth::OAuthService *> Session::oAuth()
{
  std::vector<const Auth::OAuthService *> result;
  result.reserve(myOAuthServices.size());
  for (const auto& auth : myOAuthServices) {
    result.push_back(auth.get());
  }
  return result;
}
