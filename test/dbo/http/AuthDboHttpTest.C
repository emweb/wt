/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WConfig.h>

#ifdef WT_THREADED

#include <boost/test/unit_test.hpp>

#include <Wt/WAnchor.h>
#include <Wt/WLink.h>

#include <Wt/WServer.h>
#include <Wt/WIOService.h>
#include <Wt/Http/Client.h>
#include <Wt/Http/Response.h>
#include <Wt/Http/ResponseContinuation.h>
#include <Wt/Http/Request.h>
#include <Wt/cpp17/filesystem.hpp>

#include <web/Configuration.h>

#include <Wt/Dbo/Dbo.h>
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/AuthWidget.h"
#include "Wt/Auth/HashFunction.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/Identity.h"
#include "Wt/Auth/PasswordService.h"
#include "Wt/Auth/PasswordVerifier.h"
#include "Wt/Auth/Dbo/AuthInfo.h"
#include "Wt/Auth/Dbo/UserDatabase.h"

#include "../DboFixture.h"
#include "Wt/Test/WTestEnvironment.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <fstream>
#include <iostream>


namespace dbo = Wt::Dbo;
using namespace Wt;

namespace {
  const char* TABLENAMES[] = {"test_user", "auth_info", "auth_identity", "auth_token"};

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
      myAuthService_ = std::make_unique<Auth::AuthService>();
      myPasswordService_ = std::make_unique<Auth::PasswordService>(*myAuthService_);

      std::unique_ptr<Auth::PasswordVerifier> verifier
        (new Auth::PasswordVerifier());
      verifier->addHashFunction(std::make_unique<Auth::BCryptHashFunction>(12));
      myPasswordService_->setVerifier(std::move(verifier));

      session_->mapClass<TestUser>(tablenames[0]);
      session_->mapClass<AuthInfo>(tablenames[1]);
      session_->mapClass<AuthInfo::AuthIdentityType>(tablenames[2]);
      session_->mapClass<AuthInfo::AuthTokenType>(tablenames[3]);

      users_ = std::make_unique<UserDatabase>(*session_);

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
          testUser = session_->add(std::make_unique<TestUser>());
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

  class Server : public WServer
  {
  public:
    Server() {
      int argc = 7;
      const char *argv[]
        = { "test",
            "--http-address", "127.0.0.1",
            "--http-port", "0",
            "--docroot", "."
          };
      setServerConfiguration(argc, (char **)argv);
    }

    std::string address()
    {
      return "127.0.0.1:" + std::to_string(httpPort());
    }
  };

  class Client : public Wt::WObject {
  public:
    Client()
      : done_(true)
    {
      impl_.done().connect(this, &Client::onDone);
    }

    bool get(const std::string &url)
    {
      done_ = false;
      return impl_.get(url);
    }

    void waitDone()
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      while (!done_)
        doneCondition_.wait(guard);
    }

    bool isDone()
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      return done_;
    }

    void onDone(Wt::AsioWrapper::error_code err, const Http::Message& m)
    {
      std::unique_lock<std::mutex> guard(doneMutex_);

      err_ = err;
      message_ = m;

      done_ = true;
      doneCondition_.notify_one();
    }

    Wt::AsioWrapper::error_code err() { return err_; }
    const Http::Message& message() { return message_; }

  private:
    Http::Client impl_;
    bool done_;
    std::condition_variable doneCondition_;
    std::mutex doneMutex_;

    Wt::AsioWrapper::error_code err_;
    Http::Message message_;
  };

  class TestApp : public Wt::WApplication {
  public:
    TestApp(const Wt::WEnvironment& env)
      : Wt::WApplication(env),
        f_(TABLENAMES)
    {
      internalPathChanged().connect(this, &TestApp::onInternalPathChanged);
      dbo::Session& session = *f_.session_;

      root()->addNew<Wt::Auth::AuthWidget>(f_.auth(), f_.users(), f_.login());

      {
        dbo::Transaction transaction(session);

        Auth::User guestUser = f_.users().registerNew();
        guestUser.addIdentity(Auth::Identity::LoginName, "guest");
        f_.passwordAuth().updatePassword(guestUser, "guest");
        f_.login().login(guestUser);
        transaction.commit();
      }
    }

  private:
    AuthDboFixture f_;

    void onInternalPathChanged(const std::string& internalPath)
    {
      dbo::Transaction transaction(*f_.session_);
      f_.login().logout();
      transaction.commit();
    }
  };

}

BOOST_AUTO_TEST_CASE( auth_dbo_invalidate_session_id_on_logout_test )
{
  Server server;

  server.configuration().setBootstrapMethod(Configuration::Progressive);

  WApplication* app = nullptr;
  server.addEntryPoint(Wt::EntryPointType::Application,
                       [&app](const Wt::WEnvironment& env) {
                          auto appPtr = std::make_unique<TestApp>(env);
                          app = appPtr.get();
                          return appPtr;
                       }, "/test");

  BOOST_REQUIRE(server.start());

  Client client;
  client.get("http://" + server.address() + "/test");

  client.waitDone();

  BOOST_REQUIRE(!client.err());
  BOOST_REQUIRE(client.message().status() == 200);
  BOOST_REQUIRE(app);

  WApplication* appBefore = app;

  std::string sessionId = app->sessionId();

  client.get("http://" + server.address() + "/test/test?wtd=" + sessionId);
  client.waitDone();

  BOOST_REQUIRE(!client.err());
  BOOST_REQUIRE(client.message().status() == 200);

  // check that a new application was not created
  BOOST_REQUIRE(app == appBefore);

  BOOST_TEST(sessionId != app->sessionId());
}

#endif // WT_THREADED