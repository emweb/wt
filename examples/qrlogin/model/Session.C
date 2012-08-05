/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"
#include "QRAuthService.h"
#include "QRTokenDatabase.h"

#include "Wt/Auth/AuthService"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/PasswordStrengthValidator"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/GoogleService"
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
  QRAuthService myQRService(myAuthService);
  MyOAuth myOAuthServices;
}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");

  Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
  myPasswordService.setVerifier(verifier);
  myPasswordService.setAttemptThrottlingEnabled(true);

  Wt::Auth::PasswordStrengthValidator *v 
    = new Wt::Auth::PasswordStrengthValidator();
  /* Relax these a bit -- it's not the main point of this example */
  v->setMinimumLength(Wt::Auth::PasswordStrengthValidator::TwoCharClass, 8);
  v->setMinimumLength(Wt::Auth::PasswordStrengthValidator::ThreeCharClass, 8);
  myPasswordService.setStrengthValidator(v);

  if (Wt::Auth::GoogleService::configured())
    myOAuthServices.push_back(new Wt::Auth::GoogleService(myAuthService));
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

  qrTokens_ = new QRTokenDatabase(*this);

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

const Wt::Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Wt::Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}

const QRAuthService& Session::qrAuth()
{
  return myQRService;
}

const std::vector<const Wt::Auth::OAuthService *>& Session::oAuth()
{
  return myOAuthServices;
}
