#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/RegistrationModel.h>
#include <Wt/WWidget.h>
#include <Wt/WContainerWidget.h>

#include "OAuthWidget.h"
#include "RegistrationView.h"

OAuthWidget::OAuthWidget(Session& session)
  : Wt::Auth::AuthWidget(Session::auth(),
                         session.users(),
                         session.login()),
                         session_(session)
{
}

void OAuthWidget::createLoggedInView()
{
  setTemplateText(Wt::WString("Logged in as: {1}").arg(
                    login().user().identity(Wt::Auth::Identity::LoginName)));
}

std::unique_ptr<Wt::WWidget> OAuthWidget::createRegistrationView(const Wt::Auth::Identity& id)
{
  auto w = std::make_unique<RegistrationView>(session_, this);
  auto model = createRegistrationModel();

  if (id.isValid())
    model->registerIdentified(id);
  w->setModel(std::move(model));
  return std::unique_ptr<Wt::WWidget>(w.release());
}
