#include <Wt/WLineEdit.h>

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
  auto detailsModel = std::make_unique<UserDetailsModel>(session_);
  detailsModel_ = addChild(std::move(detailsModel));
  updateView(detailsModel_);
}

std::unique_ptr<Wt::WWidget> RegistrationView::createFormWidget(Wt::WFormModel::Field field)
{
  if (field == UserDetailsModel::NameField)
    return std::make_unique<Wt::WLineEdit>();
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
