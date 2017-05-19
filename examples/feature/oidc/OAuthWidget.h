#ifndef OAUTH_WIDGET_H
#define OAUTH_WIDGET_H

#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/Login>
#include <Wt/Auth/Identity>
#include <Wt/WWidget>
#include <Wt/WContainerWidget>

#include "model/Session.h"

class OAuthWidget : public Wt::Auth::AuthWidget
{
public:
  OAuthWidget(Session& session);
  virtual Wt::WWidget *createRegistrationView(const Wt::Auth::Identity& id);
private:
  Session& session_;
};

#endif // OAUTH_WIDGET_H
