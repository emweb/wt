/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WServer>

#include <Wt/Auth/AuthModel>
#include <Wt/Auth/PasswordService>

#include "QRAuthWidget.h"
#include "model/Session.h"
#include "model/QRTokenDatabase.h"

class AuthApplication : public Wt::WApplication
{
public:
  AuthApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env),
      session_(appRoot() + "auth.db")
  {
    /*
     * For better support for a mobile device. Note this requires
     * progressive bootstrap being enabled (see wt_config.xml).
     */
    addMetaHeader("viewport",
		  "width=device-width, initial-scale=1, maximum-scale=1");

    session_.login().changed().connect(this, &AuthApplication::authEvent);

    useStyleSheet("css/style.css");
    messageResourceBundle().use(appRoot() + "templates");

    QRAuthWidget *authWidget = new QRAuthWidget(session_.login());

    Wt::Auth::AuthModel *model
      = new Wt::Auth::AuthModel(Session::auth(), session_.users(), this);

    model->addPasswordAuth(&Session::passwordAuth());
    model->addOAuth(Session::oAuth());
    authWidget->setModel(model);
    authWidget->setRegistrationEnabled(true);
    authWidget->configureQRAuth(Session::qrAuth(), session_.qrTokenDatabase());

    authWidget->processEnvironment();

    root()->addWidget(authWidget);
  }

  void authEvent() {
    if (session_.login().loggedIn())
      Wt::log("notice") << "User " << session_.login().user().id()
			<< " logged in.";
    else
      Wt::log("notice") << "User logged out.";
  }

private:
  Session session_;
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new AuthApplication(env);
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::Application, createApplication);

    Session::configureAuth();

    server.run();
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
