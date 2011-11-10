/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WServer>

#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/Login>
#include <Wt/Auth/PasswordService>

#include "model/Session.h"

class AuthApplication : public Wt::WApplication
{
public:
  AuthApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env),
      session_("auth.db")
  {
    useStyleSheet("css/style.css");

    authWidget_ = new Wt::Auth::AuthWidget(Session::auth(), session_.users(),
					   login_, root());

    authWidget_->addPasswordAuth(&Session::passwordAuth());
    authWidget_->addOAuth(Session::oAuth());
    authWidget_->setRegistrationEnabled(true);

    authWidget_->processEnvironment();
  }

private:
  Session session_;
  Wt::Auth::Login login_;
  Wt::Auth::AuthWidget *authWidget_;
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new AuthApplication(env);
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server(argv[0]);

    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::Application, createApplication);
    Session::initAuth();

    if (server.start()) {
      Wt::WServer::waitForShutdown();
      server.stop();
    }
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
