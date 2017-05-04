#include <Wt/WLineEdit>

#include "RegistrationView.h"
#include "model/Session.h"
#include "model/UserDetailsModel.h"

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

RegistrationView::RegistrationView(Session& session,
                                   Wt::Auth::AuthWidget *authWidget)
  : Wt::Auth::RegistrationWidget(authWidget),
    session_(session)
{
  setTemplateText(tr("template.registration"));
  detailsModel_ = new UserDetailsModel(session_, this);
  updateView(detailsModel_);
}

Wt::WWidget *RegistrationView::createFormWidget(Wt::WFormModel::Field field)
{
  if (field == UserDetailsModel::NameField)
    return new Wt::WLineEdit();
  else
    return Wt::Auth::RegistrationWidget::createFormWidget(field);
}

bool RegistrationView::validate()
{
  bool result = Wt::Auth::RegistrationWidget::validate();

  updateModel(detailsModel_);
  if (!detailsModel_->validate())
    result = false;
  updateView(detailsModel_);

  return result;
}

void RegistrationView::registerUserDetails(Wt::Auth::User& authUser)
{
  detailsModel_->save(authUser);
}
