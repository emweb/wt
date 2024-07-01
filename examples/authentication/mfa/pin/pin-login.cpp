#include "Wt/Auth/Mfa/AbstractMfaProcess.h"
#include "mysession.h"

#include "myauthwidget.h"

#include <Wt/Auth/AuthWidget.h>

#include <Wt/WApplication.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDateTime.h>
#include <Wt/WLocale.h>
#include <Wt/WText.h>

class PinApplication : public Wt::WApplication {
public:
  PinApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env),
      session_(appRoot() + "auth.db")
  {
    session_.configureAuth();
    session_.login().changed().connect(this, &PinApplication::authEvent);

    root()->addStyleClass("container");
    setTheme(std::make_shared<Wt::WBootstrap5Theme>());

    messageResourceBundle().use(appRoot() + "template");

    auto authWidget = std::make_unique<MyAuthWidget>(session_.auth(), session_.users(), session_.login());
    authWidget->authenticated().connect([this](Wt::Auth::Mfa::AuthenticationResult res) {
      finalAuthEvent(res);
    });

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

  void finalAuthEvent(Wt::Auth::Mfa::AuthenticationResult res) {
    if (res.status() == Wt::Auth::Mfa::AuthenticationStatus::Success) {
      const Wt::Auth::User& u = session_.login().user();
      log("notice")
        << "User " << u.id()
        << " (" << u.identity(Wt::Auth::Identity::LoginName) << ")"
        << " logged in.";
    } else {
      log("notice")
        << "User failed to log in: "
        << res.message();
    }
  }
private:
  MySession session_;
};

int main(int argc, char** argv)
{
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
    return std::make_unique<PinApplication>(env);
  });
};
