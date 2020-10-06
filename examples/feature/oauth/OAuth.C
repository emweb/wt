/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>
#include <Wt/WServer.h>
#include <Wt/WText.h>

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/GoogleService.h>

using namespace Wt;

Auth::AuthService authService;
std::unique_ptr<Auth::GoogleService> googleService = nullptr;

class OAuthApplication : public WApplication
{
public:
  OAuthApplication(const WEnvironment& env)
    : WApplication(env)
  {
    if (!googleService) {
      root()->addWidget(std::make_unique<WText>(
                    "This example requires a Google Auth service "
                    "configuration"));
      return;
    }

    process_ = googleService->createProcess
      (googleService->authenticationScope());
    auto ggi = root()->addWidget(std::make_unique<WImage>("css/oauth-google.png"));
    ggi->clicked().connect(process_.get(),
                           &Auth::OAuthProcess::startAuthenticate);

    process_->authenticated().connect(this, &OAuthApplication::authenticated);
  }

  void authenticated(const Auth::Identity& identity) {
    root()->clear();
    root()->addWidget(std::make_unique<WText>("Welcome, " + identity.name()));
  }

private:
  std::unique_ptr<Auth::OAuthProcess> process_;
};

std::unique_ptr<WApplication> createApplication(const Wt::WEnvironment& env)
{
  return std::make_unique<OAuthApplication>(env);
}

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(EntryPointType::Application, createApplication);

    if (Auth::GoogleService::configured()) {
      googleService = std::make_unique<Auth::GoogleService>(authService);
    }

    server.run();
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
