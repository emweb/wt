#include "mysession.h"

#include <Wt/Auth/AuthWidget.h>

#include <Wt/WApplication.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDateTime.h>
#include <Wt/WLocale.h>
#include <Wt/WText.h>

class TotpApplication : public Wt::WApplication {
public:
  TotpApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env),
      session_(appRoot() + "auth.db")
  {
    session_.configureAuth();
    session_.login().changed().connect(this, &TotpApplication::authEvent);

    root()->addStyleClass("container");
    setTheme(std::make_shared<Wt::WBootstrap5Theme>());

    useStyleSheet("css/style.css");

    auto authWidget = std::make_unique<Wt::Auth::AuthWidget>(
            session_.auth(), session_.users(), session_.login());

    authWidget->model()->addPasswordAuth(&session_.passwordAuth());
    authWidget->setRegistrationEnabled(true);

    authWidget->processEnvironment();

    root()->addWidget(std::move(authWidget));
  }

  void authEvent() {
    if (session_.login().loggedIn()) {
      const Wt::Auth::User& u = session_.login().user();
      log("notice")
        << "User " << u.id()
        << " (" << u.identity(Wt::Auth::Identity::LoginName) << ")"
        << " logged in.";
    } else
      log("notice") << "User logged out.";
  }
private:
  MySession session_;
};

int main(int argc, char** argv)
{
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
    return std::make_unique<TotpApplication>(env);
  });
};
