#include <string>

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

#include <Wt/Auth/AuthModel.h>
#include <Wt/Auth/AuthService.h>

#include "OAuthAuthorizationEndpoint.h"

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

OAuthAuthorizationEndpoint::OAuthAuthorizationEndpoint(const Wt::WEnvironment& env,
                                                       std::unique_ptr<Session> session)
  : Wt::WApplication(env), session_(std::move(session))
{
  messageResourceBundle().use("strings");
  messageResourceBundle().use("templates");

  auto authwidget = std::make_unique<OAuthWidget>(*session_);
  authwidget->model()->addPasswordAuth(&Session::passwordAuth());
  authwidget->setRegistrationEnabled(true);
  authwidget->processEnvironment();

  process_ = std::make_unique<Wt::Auth::OAuthAuthorizationEndpointProcess>(
      session_->login(),
      session_->users());
  process_->authorized().connect(
      process_.get(),
      &Wt::Auth::OAuthAuthorizationEndpointProcess::authorizeScope);
  process_->processEnvironment();

  if (process_->validRequest()) {
    root()->addWidget(std::move(authwidget));
  } else
    root()->addWidget(std::make_unique<Wt::WText>(Wt::WString("The request was invalid.")));
}
