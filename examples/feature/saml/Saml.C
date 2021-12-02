/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WServer.h>

#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/PasswordService.h>

#include "model/Session.h"

class SamlApplication final : public Wt::WApplication {
public:
  explicit SamlApplication(const Wt::WEnvironment &env);

private:
  Session session_;

  void authEvent();
};

SamlApplication::SamlApplication(const Wt::WEnvironment &env)
  : WApplication(env),
    session_(appRoot() + "auth.db")
{
  session_.login().changed().connect(this, &SamlApplication::authEvent);

  root()->addStyleClass("container");
  setTheme(std::make_shared<Wt::WBootstrapTheme>());

  useStyleSheet("/css/style.css");

  auto authWidget = root()->addNew<Wt::Auth::AuthWidget>(Session::auth(), session_.users(), session_.login());

  authWidget->model()->addPasswordAuth(&Session::passwordAuth());
  authWidget->model()->addSaml(Session::saml());
  authWidget->setRegistrationEnabled(true);

  authWidget->processEnvironment();
}

void SamlApplication::authEvent()
{
  if (session_.login().loggedIn()) {
    const Wt::Auth::User& u = session_.login().user();
    log("notice")
      << "User " << u.id()
      << " (" << u.identity(Wt::Auth::Identity::LoginName) << ")"
      << " logged in.";
  } else
    log("notice") << "User logged out.";
}

int main(int argc, char *argv[])
{
  try {
    Wt::WServer server{argc, argv, WTHTTP_CONFIGURATION};

    server.addEntryPoint(Wt::EntryPointType::Application, [](const Wt::WEnvironment &env) {
      return std::make_unique<SamlApplication>(env);
    });

    Session::configureAuth();

    server.run();

  } catch (Wt::WServer::Exception &e) {
    std::cerr << e.what() << '\n';
  } catch (Wt::Dbo::Exception &e) {
    std::cerr << e.what() << '\n';
  } catch (std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  Session::finalizeAuth();
}
