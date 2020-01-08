// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_UPDATE_PASSWORD_WIDGET_H_
#define WT_AUTH_UPDATE_PASSWORD_WIDGET_H_

#include <Wt/WTemplateFormView.h>
#include <Wt/Auth/RegistrationModel.h>

namespace Wt {
  namespace Auth {

class AuthModel;
class RegistrationModel;

/*! \class UpdatePasswordWidget Wt/Auth/UpdatePasswordWidget.h
 *  \brief A widget which allows a user to choose a new password.
 *
 * This widget lets a user choose a new password.
 *
 * The widget renders the <tt>"Wt.Auth.template.update-password"</tt>
 * template. Optionally, it asks for the current password, as well as
 * a new password.
 *
 * \sa AuthWidget::createUpdatePasswordView()
 *
 * \ingroup auth
 */
class WT_API UpdatePasswordWidget : public WTemplateFormView
{
public:
  /*! \brief Constructor.
   *
   * If \p authModel is not \c nullptr, the user also has to authenticate
   * first using his current password.
   */
  UpdatePasswordWidget(const User& user,
		       std::unique_ptr<RegistrationModel> registrationModel,
		       const std::shared_ptr<AuthModel>& authModel);

  /*! \brief Signal emitted when the password was updated.
   */
  Signal<>& updated() { return updated_; }

  /*! \brief Signal emitted when cancel clicked.
   */
  Signal<>& canceled() { return canceled_; }

protected:
  virtual std::unique_ptr<WWidget> createFormWidget(WFormModel::Field field)
    override;

private:
  User user_;

  std::unique_ptr<RegistrationModel> registrationModel_;
  std::shared_ptr<AuthModel> authModel_;
  Signal<> updated_;
  Signal<> canceled_;

  void checkPassword();
  void checkPassword2();
  bool validate();
  void doUpdate();
  void cancel();
};

  }
}

#endif // WT_AUTH_UPDATE_PASSWORD_WIDGET_H_
