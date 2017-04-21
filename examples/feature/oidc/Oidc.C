#include <Wt/Http/Client>
#include <Wt/WText>
#include <Wt/WApplication>
#include <Wt/Auth/OAuthService>
#include <Wt/Auth/AuthService>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/Auth/Identity>
#include <Wt/Auth/GoogleService>
#include <Wt/Auth/OidcService>

#include <Wt/Json/Serializer>
#include <Wt/Json/Object>

#include "model/Session.h"
#include "OAuthAuthorizationEndpoint.h"

const Wt::Auth::AuthService authService;
const Wt::Auth::OidcService* oidcservice_ = 0;

class myOidcService : public Wt::Auth::OidcService
{
public:

  myOidcService()
    : Wt::Auth::OidcService(authService)
  {
    std::string url = configurationProperty("application-url");

    setRedirectEndpoint(url + "/oauth2/callback");
    setClientId(configurationProperty("oauth2-client-id"));
    setClientSecret(configurationProperty("oauth2-client-secret"));

    setAuthEndpoint(url + "/oauth2/authorize");
    setTokenEndpoint(url + "/oauth2/token");
    setUserInfoEndpoint(url + "/oidc/userinfo");

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

    if (!oidcservice_)
      oidcservice_ = new myOidcService;

    process_ = oidcservice_->createProcess("email profile");

    Wt::WImage* image = new Wt::WImage("img/Wt_vol_gradient.png",root());
    image->clicked().connect(process_,&Wt::Auth::OAuthProcess::startAuthenticate);

    process_->authenticated().connect(this,&OidcClient::authenticated);
  }


private:
  Wt::Auth::OidcProcess* process_;

  void authenticated(Wt::Auth::Identity id)
  {
    root()->clear();
    new Wt::WText("Welcome, " + id.name(),root());
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
    std::string uri;
    Wt::WServer::instance()->readConfigurationProperty(
        "application-url",
        uri);
    uris.insert(uri + "/oauth2/callback");
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
  std::string dbPath = server.appRoot() + "auth.db";
  Wt::ApplicationCreator callback = boost::bind(&createAuthEndpoint, _1, dbPath);
  server.addEntryPoint(Wt::Application, callback, "/oauth2/authorize");

  server.addEntryPoint(Wt::Application, createClient);

  Session tokenSession(dbPath);
  Wt::Auth::OAuthTokenEndpoint *tokenEndpoint = new Wt::Auth::OAuthTokenEndpoint(tokenSession.users(), "https://localhost:8080/");
  server.addResource(tokenEndpoint, "/oauth2/token");

  Session userInfoSession(dbPath);
  Wt::WResource *userInfoEndpoint = new Wt::Auth::OidcUserInfoEndpoint(userInfoSession.users());
  server.addResource(userInfoEndpoint, "/oidc/userinfo");

  Session::configureAuth();

  server.run();
}
