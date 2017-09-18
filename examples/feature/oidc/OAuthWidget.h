#ifndef OAUTH_WIDGET_H
#define OAUTH_WIDGET_H

#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/Identity.h>
#include <Wt/WWidget.h>
#include <Wt/WContainerWidget.h>

#include "model/Session.h"

class OAuthWidget : public Wt::Auth::AuthWidget
{
public:
  OAuthWidget(Session& session);
  virtual void createLoggedInView() override;
  virtual std::unique_ptr<WWidget> createRegistrationView(const Wt::Auth::Identity& id) override;
private:
  Session& session_;
};

#endif // OAUTH_WIDGET_H
