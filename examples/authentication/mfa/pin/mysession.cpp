#include "mysession.h"

#include <Wt/Dbo/backend/Sqlite3.h>

#include <Wt/Auth/Mfa/TotpProcess.h>

#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/GoogleService.h>
#include <Wt/Auth/FacebookService.h>

namespace {
  Wt::Auth::AuthService myAuthService;
  Wt::Auth::PasswordService myPasswordService(myAuthService);
}

void addUser(Wt::Dbo::Session& session, UserDatabase& users, const std::string& loginName,
             const std::string& email, const std::string& password)
{
  Wt::Dbo::Transaction t(session);
  auto user = session.addNew<MyUser>(loginName);
  auto authUser = users.registerNew();
  authUser.addIdentity(Wt::Auth::Identity::LoginName, loginName);
  authUser.setEmail(email);
  myPasswordService.updatePassword(authUser, password);

  // Link myuser and auth user
  Wt::Dbo::ptr<AuthInfo> authInfo = session.find<AuthInfo>("where id = ?").bind(authUser.id());
  authInfo.modify()->setUser(user);
}

MySession::MySession(const std::string& sqliteDb)
{
  Wt::Dbo::backend::Sqlite3 *sqlite3 = new Wt::Dbo::backend::Sqlite3(sqliteDb);
  sqlite3->setDateTimeStorage(Wt::Dbo::SqlDateTimeType::Date, Wt::Dbo::backend::DateTimeStorage::JulianDaysAsReal);
  auto connection = std::unique_ptr<Wt::Dbo::SqlConnection>(sqlite3);

  connection->setProperty("show-queries", "true");
  setConnection(std::move(connection));

  mapClass<MyUser>("user");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");

  try {
    if (!created_) {
      createTables();
      created_ = true;
      Wt::log("info") << "Created database.";
    } else {
      Wt::log("info") << "Using existing database";
    }
  } catch (Wt::Dbo::Exception& e) {
    Wt::log("info") << "Using existing database";
  }
  users_ = std::make_unique<UserDatabase>(*this, &myAuthService);
}

void MySession::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  myAuthService.setEmailVerificationEnabled(true);
  myAuthService.setEmailVerificationRequired(true);

  myAuthService.setMfaProvider(Wt::Auth::Identity::MultiFactor);
  myAuthService.setMfaRequired(true);

  auto verifier = std::make_unique<Wt::Auth::PasswordVerifier>();
  verifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(7));
  myPasswordService.setVerifier(std::move(verifier));
  myPasswordService.setPasswordThrottle(std::make_unique<Wt::Auth::AuthThrottle>());
  myPasswordService.setStrengthValidator(std::make_unique<Wt::Auth::PasswordStrengthValidator>());

  if (created_) {
    addUser(*this, *users_.get(), "admin", "admin@example.com", "admin");
    addUser(*this, *users_.get(), "user", "user@example.com", "user");
  }
}

Wt::Auth::AbstractUserDatabase& MySession::users()
{
  return *users_;
}

const Wt::Auth::AuthService& MySession::auth()
{
  return myAuthService;
}

const Wt::Auth::PasswordService& MySession::passwordAuth()
{
  return myPasswordService;
}

