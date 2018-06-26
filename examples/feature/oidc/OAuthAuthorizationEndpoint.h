#include <string>

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

#include <Wt/Auth/OAuthAuthorizationEndpointProcess.h>

#include "model/OidcUserDatabase.h"
#include "model/User.h"
#include "model/Session.h"
#include "OAuthWidget.h"

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class OAuthAuthorizationEndpoint : public Wt::WApplication
{
public:
  OAuthAuthorizationEndpoint(const Wt::WEnvironment& env,
                             std::unique_ptr<Session> session);

  static Wt::WApplication *createAuthEndpoint(const Wt::WEnvironment& env,
                                              std::string dbPath);
private:
  std::unique_ptr<Session> session_;
  std::unique_ptr<Wt::Auth::OAuthAuthorizationEndpointProcess> process_;
};
