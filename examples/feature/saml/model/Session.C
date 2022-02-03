/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"

#include "../SamlService.h"

#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/PasswordStrengthValidator.h"
#include "Wt/Auth/PasswordVerifier.h"

#include "Wt/Auth/Dbo/AuthInfo.h"
#include "Wt/Auth/Saml/Service.h"

#include "Wt/Dbo/backend/Sqlite3.h"

using namespace Wt;

namespace {

Auth::AuthService myAuthService;
Auth::PasswordService myPasswordService(myAuthService);
std::vector<std::unique_ptr<Auth::Saml::Service>> mySamlServices;

}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  myAuthService.setEmailVerificationEnabled(true);
  myAuthService.setEmailVerificationRequired(true);

  std::unique_ptr<Auth::PasswordVerifier> verifier
    = std::make_unique<Auth::PasswordVerifier>();
  verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(7));
  myPasswordService.setVerifier(std::move(verifier));
  myPasswordService.setAttemptThrottlingEnabled(true);
  myPasswordService.setStrengthValidator(std::make_unique<Auth::PasswordStrengthValidator>());

  mySamlServices.push_back(std::make_unique<SamlService>(myAuthService));
}

void Session::finalizeAuth()
{
  mySamlServices.clear();
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
    std::cerr << "Created database." << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
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

const std::vector<const Auth::Saml::Service *> Session::saml()
{
  std::vector<const Auth::Saml::Service *> result;
  for (auto &auth : mySamlServices) {
    result.push_back(auth.get());
  }
  return result;
}
