#include <Wt/Auth/AuthService>
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/Login>
#include <Wt/Auth/Identity>
#include <Wt/Auth/RegistrationModel>
#include <Wt/WWidget>
#include <Wt/WContainerWidget>

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

Wt::WWidget* OAuthWidget::createRegistrationView(const Wt::Auth::Identity& id)
{
  RegistrationView *w = new RegistrationView(session_, this);
  Wt::Auth::RegistrationModel *model = createRegistrationModel();

  if (id.isValid())
    model->registerIdentified(id);
  w->setModel(model);
  return w;
}
