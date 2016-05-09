/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/AuthModel"
#include "Wt/Auth/Login"
#include "Wt/Auth/UpdatePasswordWidget"
#include "Wt/Auth/User"

#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WText"

namespace Wt {
  namespace Auth {

UpdatePasswordWidget::UpdatePasswordWidget(const User& user,
					   RegistrationModel *registrationModel,
					   AuthModel *authModel,
					   WContainerWidget *parent)
  : WTemplateFormView(tr("Wt.Auth.template.update-password"), parent),
    user_(user),
    registrationModel_(registrationModel),
    authModel_(authModel)
{
  registrationModel_->setValue(RegistrationModel::LoginNameField,
			       user.identity(Identity::LoginName));
  registrationModel_->setReadOnly(RegistrationModel::LoginNameField, true);

  if (user.password().empty())
    authModel_ = 0;
  else if (authModel_)
    authModel_->reset();

  if (authModel_ && authModel_->baseAuth()->emailVerificationEnabled()) {
    /*
     * This is set in the model so that the password checker can take
     * into account whether the password is derived from the email
     * address.
     */
    registrationModel_->setValue
      (RegistrationModel::EmailField,
       WT_USTRING::fromUTF8(user.email() + " " + user.unverifiedEmail()));
  }

  // Make sure it does not block validation
  registrationModel_->setVisible(RegistrationModel::EmailField, false);

  WPushButton *okButton = new WPushButton(tr("Wt.WMessageBox.Ok"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  if (authModel_) {
    authModel_->setValue(AuthModel::LoginNameField,
			 user.identity(Identity::LoginName));

    updateViewField(authModel_, AuthModel::PasswordField);

    authModel_->configureThrottling(okButton);

    WLineEdit *password = resolve<WLineEdit *>(AuthModel::PasswordField);
    password->setFocus(true);
  }

  updateView(registrationModel_);

  WLineEdit *password = resolve<WLineEdit *>
    (RegistrationModel::ChoosePasswordField);
  WLineEdit *password2 = resolve<WLineEdit *>
    (RegistrationModel::RepeatPasswordField);
  WText *password2Info = resolve<WText *>
    (RegistrationModel::RepeatPasswordField + std::string("-info"));

  registrationModel_->validatePasswordsMatchJS(password,
					       password2, password2Info);

  if (!authModel_)
    password->setFocus(true);

  okButton->clicked().connect(this, &UpdatePasswordWidget::doUpdate);
  cancelButton->clicked().connect(this, &UpdatePasswordWidget::close);

  bindWidget("ok-button", okButton);
  bindWidget("cancel-button", cancelButton);

}

WWidget *UpdatePasswordWidget::createFormWidget(WFormModel::Field field)
{
  WFormWidget *result = 0;

  if (field == RegistrationModel::LoginNameField) {
    result = new WLineEdit();
  } else if (field == AuthModel::PasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(WLineEdit::Password);
    result = p;
  } else if (field == RegistrationModel::ChoosePasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(WLineEdit::Password);
    p->keyWentUp().connect
      (boost::bind(&UpdatePasswordWidget::checkPassword, this));
    p->changed().connect
      (boost::bind(&UpdatePasswordWidget::checkPassword, this));
    result = p;
  } else if (field == RegistrationModel::RepeatPasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(WLineEdit::Password);
    p->changed().connect
      (boost::bind(&UpdatePasswordWidget::checkPassword2, this));
    result = p;
  }

  return result;
}

void UpdatePasswordWidget::checkPassword()
{
  updateModelField(registrationModel_, RegistrationModel::ChoosePasswordField);
  registrationModel_->validateField(RegistrationModel::ChoosePasswordField);
  updateViewField(registrationModel_, RegistrationModel::ChoosePasswordField);
}

void UpdatePasswordWidget::checkPassword2()
{
  updateModelField(registrationModel_, RegistrationModel::RepeatPasswordField);
  registrationModel_->validateField(RegistrationModel::RepeatPasswordField);
  updateViewField(registrationModel_, RegistrationModel::RepeatPasswordField);
}

bool UpdatePasswordWidget::validate()
{
  bool valid = true;

  if (authModel_) {
    updateModelField(authModel_, AuthModel::PasswordField);

    if (!authModel_->validate()) {
      updateViewField(authModel_, AuthModel::PasswordField);
      valid = false;
    }
  }

  registrationModel_->validateField(RegistrationModel::LoginNameField);
  checkPassword();
  checkPassword2();

  registrationModel_->validateField(RegistrationModel::EmailField);

  if (!registrationModel_->valid()) 
    valid = false;

  return valid;
}

void UpdatePasswordWidget::doUpdate()
{
  if (validate()) {
    WT_USTRING password
      = registrationModel_->valueText(RegistrationModel::ChoosePasswordField);
    registrationModel_->passwordAuth()->updatePassword(user_, password);
    registrationModel_->login().login(user_);

    close();
  }
}

void UpdatePasswordWidget::close()
{
  delete this;
}

  }
}
