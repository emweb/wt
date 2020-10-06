/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WServer.h>

#include <Wt/Auth/AuthModel.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/RegistrationModel.h>

#include "QRAuthWidget.h"
#include "model/Session.h"
#include "model/QRTokenDatabase.h"

using namespace Wt;

class AuthApplication : public WApplication
{
public:
  AuthApplication(const WEnvironment& env)
    : WApplication(env),
      session_(appRoot() + "auth.db")
  {
    /*
     * For better support for a mobile device. Note this requires
     * progressive bootstrap being enabled (see wt_config.xml).
     */
    addMetaHeader(MetaHeaderType::Meta,
                  "width=device-width", "initial-scale=1", "maximum-scale=1");

    session_.login().changed().connect(this, &AuthApplication::authEvent);

    useStyleSheet("css/style.css");
    messageResourceBundle().use(appRoot() + "templates");

    auto authWidget = std::make_unique<QRAuthWidget>(session_.login());

    auto model = std::make_unique<Auth::AuthModel>(Session::auth(), session_.users());

    model->addPasswordAuth(&Session::passwordAuth());
    model->addOAuth(Session::oAuth());
    authWidget->setModel(std::move(model));
    authWidget->setRegistrationEnabled(true);
    authWidget->configureQRAuth(Session::qrAuth(), session_.qrTokenDatabase());

    authWidget->processEnvironment();

    root()->addWidget(std::move(authWidget));
  }

  void authEvent() {
    if (session_.login().loggedIn())
      log("notice") << "User " << session_.login().user().id()
			<< " logged in.";
    else
      log("notice") << "User logged out.";
  }

private:
  Session session_;
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<AuthApplication>(env);
}

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(EntryPointType::Application, &createApplication);

    Session::configureAuth();

    server.run();
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }
}
