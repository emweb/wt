/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
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
#include "Wt/Auth/Dbo/AuthInfo"
#include "Wt/Auth/Dbo/UserDatabase"

#include <Wt/WApplication>
#include <Wt/WLogger>

#ifndef WT_WIN32
#include <unistd.h>
#endif

#if !defined(WT_WIN32) && !defined(__CYGWIN__) && !defined(ANDROID)
#define HAVE_CRYPT
#endif

using namespace Wt;
namespace dbo = Wt::Dbo;

namespace {

#ifdef HAVE_CRYPT
class UnixCryptHashFunction : public Auth::HashFunction
  {
  public:
    virtual std::string compute(const std::string& msg, 
				const std::string& salt) const
    {
      std::string md5Salt = "$1$" + salt;
      return crypt(msg.c_str(), md5Salt.c_str());
    }

    virtual bool verify(const std::string& msg,
			const std::string& salt,
			const std::string& hash) const
    {
      return crypt(msg.c_str(), hash.c_str()) == hash;
    }

    virtual std::string name () const {
      return "crypt";
    }
  };
#endif // HAVE_CRYPT

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
  MyOAuth myOAuthServices;
}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "hangmancookie");
  myAuthService.setEmailVerificationEnabled(true);

  Auth::PasswordVerifier *verifier = new Auth::PasswordVerifier();
  verifier->addHashFunction(new Auth::BCryptHashFunction(7));

#ifdef HAVE_CRYPT
  // We want to still support users registered in the pre - Wt::Auth
  // version of the hangman example
  verifier->addHashFunction(new UnixCryptHashFunction());
#endif

  myPasswordService.setVerifier(verifier);
  myPasswordService.setStrengthValidator(new Auth::PasswordStrengthValidator());
  myPasswordService.setAttemptThrottlingEnabled(true);

  if (Auth::GoogleService::configured())
    myOAuthServices.push_back(new Auth::GoogleService(myAuthService));
}

Session::Session()
  : sqlite3_(WApplication::instance()->appRoot() + "hangman.db")
{
  session_.setConnection(sqlite3_);
  sqlite3_.setProperty("show-queries", "true");

  session_.mapClass<User>("user");
  session_.mapClass<AuthInfo>("auth_info");
  session_.mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  session_.mapClass<AuthInfo::AuthTokenType>("auth_token");

  users_ = new UserDatabase(session_);

  dbo::Transaction transaction(session_);
  try {
    session_.createTables();

    /*
     * Add a default guest/guest account
     */
    Auth::User guestUser = users_->registerNew();
    guestUser.addIdentity(Auth::Identity::LoginName, "guest");
    myPasswordService.updatePassword(guestUser, "guest");

    Wt::log("info") << "Database created";
  } catch (...) {
    Wt::log("info") << "Using existing database";
  }

  transaction.commit();
}

Session::~Session()
{
  delete users_;
}

dbo::ptr<User> Session::user() const
{
  if (login_.loggedIn()) {
    dbo::ptr<AuthInfo> authInfo = users_->find(login_.user());
    dbo::ptr<User> user = authInfo->user();

    if (!user) {
      user = session_.add(new User());
      authInfo.modify()->setUser(user);
    }

    return user;
  } else
    return dbo::ptr<User>();
}

std::string Session::userName() const
{
  if (login_.loggedIn())
    return login_.user().identity(Auth::Identity::LoginName).toUTF8();
  else
    return std::string();
}

void Session::addToScore(int s)
{
  dbo::Transaction transaction(session_);

  dbo::ptr<User> u = user();
  if (u) {
    u.modify()->score += s;
    ++u.modify()->gamesPlayed;
    u.modify()->lastGame = WDateTime::currentDateTime();
  }

  transaction.commit();
}

std::vector<User> Session::topUsers(int limit)
{
  dbo::Transaction transaction(session_);

  Users top = session_.find<User>().orderBy("score desc").limit(limit);

  std::vector<User> result;
  for (Users::const_iterator i = top.begin(); i != top.end(); ++i) {
    dbo::ptr<User> user = *i;
    result.push_back(*user);
 
    dbo::ptr<AuthInfo> auth = *user->authInfos.begin();
    std::string name = auth->identity(Auth::Identity::LoginName).toUTF8();

    result.back().name = name;
  }

  transaction.commit();

  return result;
}

int Session::findRanking()
{
  dbo::Transaction transaction(session_);
  
  dbo::ptr<User> u = user();
  int ranking = -1;

  if (u)
    ranking = session_.query<int>("select distinct count(score) from user")
      .where("score > ?").bind(u->score);

  transaction.commit();
  
  return ranking + 1;
}

Auth::AbstractUserDatabase& Session::users()
{
  return *users_;
}

const Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Auth::AbstractPasswordService& Session::passwordAuth()
{
  return myPasswordService;
}

const std::vector<const Auth::OAuthService *>& Session::oAuth()
{
  return myOAuthServices;
}
