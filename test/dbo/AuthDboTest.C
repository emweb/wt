/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/Identity.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/PasswordVerifier.h"
#include "Wt/Auth/Dbo/AuthInfo.h"
#include "Wt/Auth/Dbo/UserDatabase.h"

#include "DboFixture.h"


namespace dbo = Wt::Dbo;
using namespace Wt;

class TestUser;
typedef Auth::Dbo::AuthInfo<TestUser> AuthInfo;
typedef dbo::collection< dbo::ptr<TestUser> > TestUsers;

class TestUser : public dbo::Dbo<TestUser>
{
public:
  TestUser() { }

  std::string nickname;
  dbo::collection< dbo::ptr<AuthInfo> > authInfos;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, nickname, "nickname");
    dbo::hasMany(a, authInfos, dbo::ManyToOne, "user");
  }
};

typedef Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

struct AuthDboFixture : DboFixtureBase
{
  AuthDboFixture(const char* tablenames[], const char* schema = "") :
    DboFixtureBase(), schema_(schema)
  {
    myAuthService_ = cpp14::make_unique<Auth::AuthService>();
    myPasswordService_ = cpp14::make_unique<Auth::PasswordService>(*myAuthService_);

    std::unique_ptr<Auth::PasswordVerifier> verifier
      (new Auth::PasswordVerifier());
    verifier->addHashFunction(cpp14::make_unique<Auth::BCryptHashFunction>(7));
    myPasswordService_->setVerifier(std::move(verifier));

    session_->mapClass<TestUser>(tablenames[0]);
    session_->mapClass<AuthInfo>(tablenames[1]);
    session_->mapClass<AuthInfo::AuthIdentityType>(tablenames[2]);
    session_->mapClass<AuthInfo::AuthTokenType>(tablenames[3]);

    users_ = cpp14::make_unique<UserDatabase>(*session_);

    try {
      dbo::Transaction transaction(*session_);
      if (!schema_.empty())
        session_->execute("drop schema \"" + schema_ + "\" cascade");
      else {
        session_->dropTables(); //todo:remove
      }
    } catch (...) {
    }
    std::cout << "-------------------------- end of drop ----------------------*********" << std::endl;

    dbo::Transaction transaction(*session_);
    if (!schema_.empty())
      session_->execute("create schema \"" + schema_ + "\"");

    session_->createTables();
    transaction.commit();
  }

  ~AuthDboFixture() {
    try {
      dbo::Transaction transaction(*session_);
      if (!schema_.empty())
        session_->execute("drop schema \"" + schema_ + "\" cascade");
    } catch (...) {
    }
  }

  Auth::AbstractUserDatabase& users() { return *users_; }
  Auth::Login& login() { return login_; }

  const Auth::AuthService& auth() { return *myAuthService_; }
  const Auth::AbstractPasswordService& passwordAuth() { return *myPasswordService_; }
  const std::vector<const Auth::OAuthService *>& oAuth() { return myOAuthServices; }

  dbo::ptr<TestUser> testUser() const
  {
    if (login_.loggedIn()) {
      dbo::ptr<AuthInfo> authInfo = users_->find(login_.user());
      dbo::ptr<TestUser> testUser = authInfo->user();
      if (!testUser) {
        testUser = session_->add(Wt::cpp14::make_unique<TestUser>());
        authInfo.modify()->setUser(testUser);
      }
      return testUser;
    } else
      return dbo::ptr<TestUser>();
  }

  std::unique_ptr<Auth::AuthService> myAuthService_;
  std::unique_ptr<Auth::PasswordService> myPasswordService_;
  std::vector<const Auth::OAuthService *> myOAuthServices;

  std::unique_ptr<UserDatabase> users_;
  Auth::Login login_;
  std::string schema_;
};

BOOST_AUTO_TEST_CASE( auth_dbo_test1 )
{
  const char* tablenames[] = {"test_user", "auth_info", "auth_identity", "auth_token"};
  AuthDboFixture f(tablenames);

  dbo::Session& session = *f.session_;
  {
    dbo::Transaction transaction(session);

    Auth::User guestUser = f.users().registerNew();
    guestUser.addIdentity(Auth::Identity::LoginName, "guest");
    f.passwordAuth().updatePassword(guestUser, "guest");
    f.login().login(guestUser);
    f.testUser().modify()->nickname = "anonymous";

    transaction.commit();
  }
  {
    dbo::Transaction transaction(session);

    session.rereadAll();
    BOOST_REQUIRE (f.login().user().identity(Auth::Identity::LoginName).toUTF8() == "guest");
    BOOST_REQUIRE (f.testUser()->nickname == "anonymous");

    transaction.commit();
  }
}

BOOST_AUTO_TEST_CASE( auth_dbo_test2 )
{
#ifdef POSTGRES
  const char* tablenames[] = {"test_schema.test_user", "test_schema.auth_info",
    "test_schema.auth_identity", "test_schema.auth_token"};
  AuthDboFixture f(tablenames, "test_schema");

  dbo::Session& session = *f.session_;
  {
    dbo::Transaction transaction(session);

    Auth::User guestUser = f.users().registerNew();
    guestUser.addIdentity(Auth::Identity::LoginName, "guest");
    f.passwordAuth().updatePassword(guestUser, "guest");
    f.login().login(guestUser);
    f.testUser().modify()->nickname = "anonymous";

    transaction.commit();
  }
  {
    dbo::Transaction transaction(session);

    session.rereadAll();
    BOOST_REQUIRE (f.login().user().identity(Auth::Identity::LoginName).toUTF8() == "guest");
    BOOST_REQUIRE (f.testUser()->nickname == "anonymous");

    transaction.commit();
  }
#endif //POSTGRES
}
