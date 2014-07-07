/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WServer>
#include <Wt/WText>

#include <Wt/Auth/AuthService>
#include <Wt/Auth/GoogleService>

Wt::Auth::AuthService authService;
Wt::Auth::GoogleService *googleService = 0;

class OAuthApplication : public Wt::WApplication
{
public:
  OAuthApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
  {
    if (!googleService) {
      new Wt::WText("This example requires a Google Auth service "
		    "configuration", root());
      return;
    }

    process_ = googleService->createProcess
      (googleService->authenticationScope());
    Wt::WImage *ggi = new Wt::WImage("css/oauth-google.png", root());  
    ggi->clicked().connect(process_,
			   &Wt::Auth::OAuthProcess::startAuthenticate);

    process_->authenticated().connect(this, &OAuthApplication::authenticated);
  }

  void authenticated(const Wt::Auth::Identity& identity) {
    root()->clear();
    new Wt::WText("Welcome, " + identity.name(), root());
  }

private:
  Wt::Auth::OAuthProcess* process_;
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new OAuthApplication(env);
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::Application, createApplication);

    if (Wt::Auth::GoogleService::configured()) {
      googleService = new Wt::Auth::GoogleService(authService);
    }

    server.run();
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
