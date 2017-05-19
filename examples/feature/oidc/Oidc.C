#include <Wt/WText>
#include <Wt/WApplication>
#include <Wt/WServer>
#include <Wt/WContainerWidget>
#include <Wt/WImage>

#include <Wt/Auth/AuthService>
#include <Wt/Auth/Identity>
#include <Wt/Auth/OAuthTokenEndpoint>
#include <Wt/Auth/OidcService>
#include <Wt/Auth/OidcUserInfoEndpoint>

#include "model/Session.h"
#include "OAuthAuthorizationEndpoint.h"

namespace {
  const Wt::Auth::AuthService authService;
  const Wt::Auth::OidcService* oidcService = 0;
  std::string deployUrl;
}

class MyOidcService : public Wt::Auth::OidcService
{
public:

  MyOidcService()
    : Wt::Auth::OidcService(authService)
  {
    setRedirectEndpoint(deployUrl + "/oauth2/callback");
    setClientId(configurationProperty("oauth2-client-id"));
    setClientSecret(configurationProperty("oauth2-client-secret"));

    setAuthEndpoint(deployUrl + "/oauth2");
    setTokenEndpoint(deployUrl + "/oauth2/token");
    setUserInfoEndpoint(deployUrl + "/oidc/userinfo");

    setName("oidc");
    setDescription("OpenID Connect");
  }

};

class OidcClient : public Wt::WApplication
{
public:
  OidcClient(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
  {
    setTitle("OIDC Client Example");

    process_ = oidcService->createProcess("email profile");

    Wt::WImage* image = new Wt::WImage("img/Wt_vol_gradient.png", root());
    image->clicked().connect(process_, &Wt::Auth::OAuthProcess::startAuthenticate);

    process_->authenticated().connect(this, &OidcClient::authenticated);
  }


private:
  Wt::Auth::OidcProcess* process_;

  void authenticated(Wt::Auth::Identity id)
  {
    root()->clear();
    new Wt::WText("Welcome, " + id.name(), root());
  }

  virtual ~OidcClient()
  {
    delete process_;
  }
};

Wt::WApplication *createAuthEndpoint(const Wt::WEnvironment& env, std::string dbPath)
{
  Session *session = new Session(dbPath);

  // add an example client
  if (!session->users().idpClientFindWithId("example_client_id").checkValid()) {
    std::set<std::string> uris;
    uris.insert(deployUrl + "/oauth2/callback");
    session->users().idpClientAdd(
        "example_client_id",
        true,
        uris,
        Wt::Auth::HttpAuthorizationBasic,
        "example_client_secret");
  }

  return new OAuthAuthorizationEndpoint(env, session);
}

Wt::WApplication *createClient(const Wt::WEnvironment& env)
{
  return new OidcClient(env);
}

int main(int argc, char** argv)
{
  Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);
  server.readConfigurationProperty("application-url",deployUrl);

  MyOidcService myOidcService;
  oidcService = &myOidcService;

  std::string dbPath = server.appRoot() + "auth.db";
  Wt::ApplicationCreator callback = boost::bind(&createAuthEndpoint, _1, dbPath);
  server.addEntryPoint(Wt::Application, callback, "/oauth2");

  server.addEntryPoint(Wt::Application, createClient);

  Session tokenSession(dbPath);
  Wt::Auth::OAuthTokenEndpoint *tokenEndpoint =
    new Wt::Auth::OAuthTokenEndpoint(tokenSession.users(), deployUrl);
  server.addResource(tokenEndpoint, "/oauth2/token");

  Session userInfoSession(dbPath);
  Wt::WResource *userInfoEndpoint = new Wt::Auth::OidcUserInfoEndpoint(userInfoSession.users());
  server.addResource(userInfoEndpoint, "/oidc/userinfo");

  Session::configureAuth();

  server.run();
}
