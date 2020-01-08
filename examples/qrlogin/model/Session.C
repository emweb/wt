/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"
#include "QRAuthService.h"
#include "QRTokenDatabase.h"

#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/PasswordStrengthValidator.h"
#include "Wt/Auth/PasswordVerifier.h"
#include "Wt/Auth/GoogleService.h"
#include "Wt/Auth/Dbo/AuthInfo.h"
#include "Wt/Auth/Dbo/UserDatabase.h"

#include "Wt/Dbo/backend/Sqlite3.h"

namespace {

  class MyOAuth : public std::vector<const Auth::OAuthService *>
  {
  public:
    ~MyOAuth()
    {
      for (unsigned i = 0; i < size(); ++i)
        delete (*this)[i];
    }
  };

  Auth::AuthService myAuthService;
  Auth::PasswordService myPasswordService(myAuthService);
  QRAuthService myQRService(myAuthService);
  MyOAuth myOAuthServices;
}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");

  auto verifier = cpp14::make_unique<Auth::PasswordVerifier>();
  verifier->addHashFunction(cpp14::make_unique<Auth::BCryptHashFunction>(7));
  myPasswordService.setVerifier(std::move(verifier));
  myPasswordService.setAttemptThrottlingEnabled(true);

  auto validator
    = cpp14::make_unique<Auth::PasswordStrengthValidator>();
  /* Relax these a bit -- it's not the main point of this example */
  validator->setMinimumLength(Auth::PasswordStrengthType::TwoCharClass, 8);
  validator->setMinimumLength(Auth::PasswordStrengthType::ThreeCharClass, 8);
  myPasswordService.setStrengthValidator(std::move(validator));

  if (Auth::GoogleService::configured())
    myOAuthServices.push_back(new Auth::GoogleService(myAuthService));
}

Session::Session(const std::string& sqliteDb)
{
  auto connection = cpp14::make_unique<Dbo::backend::Sqlite3>(sqliteDb);
  connection->setProperty("show-queries", "true");
  setConnection(std::move(connection));

  mapClass<User>("user");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");

  qrTokens_ = cpp14::make_unique<QRTokenDatabase>(*this);

  try {
    createTables();
    std::cerr << "Created database." << std::endl;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
  }

  users_ = cpp14::make_unique<UserDatabase>(*this);
}

Session::~Session()
{
  users_.reset();
}

Auth::AbstractUserDatabase& Session::users()
{
  return *users_;
}

QRTokenDatabase& Session::qrTokenDatabase()
{
  return *qrTokens_;
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

const QRAuthService& Session::qrAuth()
{
  return myQRService;
}

const std::vector<const Auth::OAuthService *>& Session::oAuth()
{
  return myOAuthServices;
}
