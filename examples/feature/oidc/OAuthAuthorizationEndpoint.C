#include <string>

#include <Wt/WContainerWidget>
#include <Wt/WText>

#include <Wt/Auth/AuthModel>
#include <Wt/Auth/AuthService>

#include "OAuthAuthorizationEndpoint.h"

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

OAuthAuthorizationEndpoint::OAuthAuthorizationEndpoint(const Wt::WEnvironment& env, Session* session)
  : Wt::WApplication(env), session_(session)
{
  messageResourceBundle().use("strings");
  messageResourceBundle().use("templates");

  OAuthWidget* authwidget = new OAuthWidget(*session);
  authwidget->model()->addPasswordAuth(&Session::passwordAuth());
  authwidget->setRegistrationEnabled(true);
  authwidget->processEnvironment();

  process_ = new Wt::Auth::OAuthAuthorizationEndpointProcess(
      session->login(),
      session->users());
  process_->authorized().connect(
      process_,
      &Wt::Auth::OAuthAuthorizationEndpointProcess::authorizeScope);
  process_->processEnvironment();

  if (process_->validRequest()) {
    root()->addWidget(authwidget);
  } else
    new Wt::WText("The request was invalid.", root());
}

OAuthAuthorizationEndpoint::~OAuthAuthorizationEndpoint()
{
  delete session_;
  delete process_;
}
