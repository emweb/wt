#ifndef REGISTRATION_VIEW_H
#define REGISTRATION_VIEW_H

#include <Wt/Auth/RegistrationWidget.h>

class UserDetailsModel;
class Session;

class RegistrationView : public Wt::Auth::RegistrationWidget
{
public:
  RegistrationView(Session& session, Wt::Auth::AuthWidget *authWidget);
  virtual std::unique_ptr<WWidget> createFormWidget(Wt::WFormModel::Field field);
protected:
  virtual bool validate();
  virtual void registerUserDetails(Wt::Auth::User& authUser);
private:
  Session& session_;
  UserDetailsModel *detailsModel_;
};

#endif // REGISTRATION_VIEW_H
