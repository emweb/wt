#include <string>

#include <Wt/WServer>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WResource>
#include <Wt/WContainerWidget>
#include <Wt/WText>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Session>
#include <Wt/Dbo/backend/Sqlite3>

#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/AuthModel>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/OAuthAuthorizationEndpointProcess>
#include <Wt/Auth/OAuthTokenEndpoint>
#include <Wt/Auth/OidcUserInfoEndpoint>

#include "model/OidcUserDatabase.h"
#include "model/User.h"
#include "model/Session.h"
#include "OAuthWidget.h"

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class OAuthAuthorizationEndpoint : public Wt::WApplication
{
public:
  OAuthAuthorizationEndpoint(const Wt::WEnvironment& env, Session* session);

  virtual ~OAuthAuthorizationEndpoint();

  static Wt::WApplication *createAuthEndpoint(const Wt::WEnvironment& env,
                                              std::string dbPath);
private:
  Session *session_;
  Wt::Auth::OAuthAuthorizationEndpointProcess* process_;
};
