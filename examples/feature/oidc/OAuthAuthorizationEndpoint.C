#include <string>

#include <Wt/WContainerWidget>
#include <Wt/WText>

#include <Wt/Auth/AuthModel>
#include <Wt/Auth/AuthService>

#include "OAuthAuthorizationEndpoint.h"

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

OAuthAuthorizationEndpoint::OAuthAuthorizationEndpoint(const Wt::WEnvironment& env,
                                                       std::unique_ptr<Session> session)
  : Wt::WApplication(env), session_(std::move(session))
{
  messageResourceBundle().use("strings");
  messageResourceBundle().use("templates");

  auto authwidget = Wt::cpp14::make_unique<OAuthWidget>(*session_);
  authwidget->model()->addPasswordAuth(&Session::passwordAuth());
  authwidget->setRegistrationEnabled(true);
  authwidget->processEnvironment();

  process_ = Wt::cpp14::make_unique<Wt::Auth::OAuthAuthorizationEndpointProcess>(
      session->login(),
      session->users());
  process_->authorized().connect(
      process_.get(),
      &Wt::Auth::OAuthAuthorizationEndpointProcess::authorizeScope);
  process_->processEnvironment();

  if (process_->validRequest()) {
    root()->addWidget(std::move(authwidget));
  } else
    root()->addWidget(Wt::cpp14::make_unique<Wt::WText>(Wt::utf8("The request was invalid.")));
}
