/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Auth/Dbo/AuthInfo.h"
#include "Wt/Auth/Dbo/UserDatabase.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/AuthThrottle.h>
#include "Wt/Auth/Identity.h"
#include <Wt/Auth/PasswordService.h>

#include "../dbo/DboFixture.h"

#include <thread>

using namespace Wt;

class TestUser;
typedef Auth::Dbo::AuthInfo<TestUser> AuthInfo;
typedef Wt::Dbo::collection<Wt::Dbo::ptr<TestUser>> TestUsers;

class TestUser : public Wt::Dbo::Dbo<TestUser>
{
public:
  TestUser() { }

  Wt::Dbo::collection<Wt::Dbo::ptr<AuthInfo>> authInfos;

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::hasMany(a, authInfos, Wt::Dbo::ManyToOne, "user");
  }
};

typedef Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

struct PasswordDboFixture : DboFixtureBase
{
  PasswordDboFixture()
    : DboFixtureBase()
  {
    myAuthService_ = std::make_unique<Auth::AuthService>();
    myPasswordService_ = std::make_unique<Auth::PasswordService>(*myAuthService_);

    session_->mapClass<TestUser>("user");
    session_->mapClass<AuthInfo>("auth_info");
    session_->mapClass<AuthInfo::AuthIdentityType>("auth_identity");
    session_->mapClass<AuthInfo::AuthTokenType>("auth_token");

    users_ = std::make_unique<UserDatabase>(*session_);

    try {
      Wt::Dbo::Transaction transaction(*session_);
      session_->dropTables();
    } catch (...) {
    }

    Wt::Dbo::Transaction transaction(*session_);
    session_->createTables();
    transaction.commit();
  }

  std::unique_ptr<Auth::AuthService> myAuthService_;
  std::unique_ptr<Auth::PasswordService> myPasswordService_;

  std::unique_ptr<UserDatabase> users_;
};

BOOST_AUTO_TEST_CASE( throttle_not_enabled_test )
{
  PasswordDboFixture f;

  Wt::Dbo::Transaction transaction(*f.session_);
  Auth::User user = f.users_->registerNew();
  transaction.commit();

  BOOST_REQUIRE(f.myPasswordService_->delayForNextAttempt(user) == 0);
}

BOOST_AUTO_TEST_CASE( throttle_enabled_no_failure_test )
{
  PasswordDboFixture f;
  f.myPasswordService_->setPasswordThrottle(std::make_unique<Wt::Auth::AuthThrottle>());

  Wt::Dbo::Transaction transaction(*f.session_);
  Auth::User user = f.users_->registerNew();
  transaction.commit();
  user.setAuthenticated(true);
  transaction.commit();

  BOOST_REQUIRE(f.myPasswordService_->delayForNextAttempt(user) == 0);
}

BOOST_AUTO_TEST_CASE( throttle_enabled_failure_test )
{
  PasswordDboFixture f;
  f.myPasswordService_->setPasswordThrottle(std::make_unique<Wt::Auth::AuthThrottle>());

  Wt::Dbo::Transaction transaction(*f.session_);
  Auth::User user = f.users_->registerNew();
  transaction.commit();

  std::vector<int> attemptResults { 1, 5, 10, 25, 25, 25, 25, 25, 25, 25 };

  // Test for 10 failure attempts
  for (std::size_t failures = 0; failures < 10; ++failures) {
    auto start = std::chrono::system_clock::now();
    // Have one additional login failure
    user.setAuthenticated(false);
    transaction.commit();

    int delay = f.myPasswordService_->delayForNextAttempt(user);
    auto end = std::chrono::system_clock::now();
    int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(end - start).count());
    elapsed++; // Add 1 second because it was rounded down

    BOOST_REQUIRE(attemptResults[failures] - elapsed <= delay);
    BOOST_REQUIRE(delay <= attemptResults[failures]);
  }
}

