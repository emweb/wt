#include <string>

#include <Wt/WApplication>
#include <Wt/WEnvironment>

#include <Wt/Auth/OAuthAuthorizationEndpointProcess>

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
