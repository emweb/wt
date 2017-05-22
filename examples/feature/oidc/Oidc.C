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

    Wt::WImage *image =
      root()->addWidget(Wt::cpp14::make_unique<Wt::WImage>("img/Wt_vol_gradient.png"));
    image->clicked().connect(process_.get(), &Wt::Auth::OAuthProcess::startAuthenticate);

    process_->authenticated().connect(this, &OidcClient::authenticated);
  }


private:
  std::unique_ptr<Wt::Auth::OAuthProcess> process_;

  void authenticated(Wt::Auth::Identity id)
  {
    root()->clear();
    root()->addWidget(Wt::cpp14::make_unique<Wt::WText>(
          Wt::WString("Welcome, {1}").arg(id.name()), Wt::TextFormat::Plain));
  }
};

int main(int argc, char** argv)
{
  Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);
  server.readConfigurationProperty("application-url",deployUrl);

  MyOidcService myOidcService;
  oidcService = &myOidcService;

  std::string dbPath = server.appRoot() + "auth.db";
  Wt::ApplicationCreator callback = [dbPath](const Wt::WEnvironment &env) {
    auto session = Wt::cpp14::make_unique<Session>(dbPath);

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

    return Wt::cpp14::make_unique<OAuthAuthorizationEndpoint>(env, std::move(session));
  };
  server.addEntryPoint(Wt::EntryPointType::Application, callback, "/oauth2");

  server.addEntryPoint(Wt::EntryPointType::Application, [](const Wt::WEnvironment& env) {
    return Wt::cpp14::make_unique<OidcClient>(env);
  });

  Session tokenSession(dbPath);
  Wt::Auth::OAuthTokenEndpoint tokenEndpoint{tokenSession.users(), deployUrl};
  server.addResource(&tokenEndpoint, "/oauth2/token");

  Session userInfoSession(dbPath);
  Wt::Auth::OidcUserInfoEndpoint userInfoEndpoint{userInfoSession.users()};
  server.addResource(&userInfoEndpoint, "/oidc/userinfo");

  Session::configureAuth();

  server.run();
}
