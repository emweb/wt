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

std::unique_ptr<Wt::WWidget> OAuthWidget::createRegistrationView(const Wt::Auth::Identity& id)
{
  auto w = Wt::cpp14::make_unique<RegistrationView>(session_, this);
  auto model = createRegistrationModel();

  if (id.isValid())
    model->registerIdentified(id);
  w->setModel(std::move(model));
  return std::unique_ptr<Wt::WWidget>(w.release());
}
