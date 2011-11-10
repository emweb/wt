/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"
#include "User.h"

#include "Wt/Auth/AuthService"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/GoogleService"
#include "Wt/Auth/Dbo/AuthInfo"
#include "Wt/Auth/Dbo/UserDatabase"

using namespace Wt;

namespace {
  class MyBaseAuth : public Auth::AuthService
  {
  public:
    MyBaseAuth() {
      setAuthTokensEnabled(true, "logincookie");
      setEmailVerificationEnabled(true);
    }
  };

  class MyPasswords : public Auth::PasswordService
  {
  public:
    MyPasswords(const Auth::AuthService& baseAuth)
      : Auth::PasswordService(baseAuth) {
      Auth::PasswordVerifier *verifier = new Auth::PasswordVerifier();

      verifier->addHashFunction(new Auth::BCryptHashFunction(7));

#ifdef WT_WITH_SSL
      verifier->addHashFunction(new Auth::SHA1HashFunction());
#endif

      setVerifier(verifier);
      setAttemptThrottlingEnabled(true);
    }
  };

  class MyOAuth : public std::vector<const Auth::OAuthService *>
  {
  public:
    ~MyOAuth()
    {
      for (unsigned i = 0; i < size(); ++i)
	delete (*this)[i];
    }
  };

  MyBaseAuth myBaseAuth;
  MyPasswords myPasswords(myBaseAuth);
  MyOAuth myOAuth;
}

void Session::initAuth()
{
  if (Auth::GoogleService::configured())
    myOAuth.push_back(new Auth::GoogleService(myBaseAuth));
}

Session::Session(const std::string& sqliteDb)
  : connection_(sqliteDb)
{
  users_ = new UserDatabase(*this);
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
  return myBaseAuth;
}

const Auth::PasswordService& Session::passwordAuth()
{
  return myPasswords;
}

const std::vector<const Auth::OAuthService *>& Session::oAuth()
{
  return myOAuth;
}
