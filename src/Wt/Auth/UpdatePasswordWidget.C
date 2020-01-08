/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService.h"
#include "Wt/Auth/AuthModel.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/UpdatePasswordWidget.h"
#include "Wt/Auth/User.h"

#include "Wt/WLineEdit.h"
#include "Wt/WPushButton.h"
#include "Wt/WText.h"

namespace Wt {
  namespace Auth {

UpdatePasswordWidget
::UpdatePasswordWidget(const User& user,
		       std::unique_ptr<RegistrationModel> registrationModel,
		       const std::shared_ptr<AuthModel>& authModel)
  : WTemplateFormView(tr("Wt.Auth.template.update-password")),
    user_(user),
    registrationModel_(std::move(registrationModel)),
    authModel_(authModel)
{
  registrationModel_->setValue(RegistrationModel::LoginNameField,
			       user.identity(Identity::LoginName));
  registrationModel_->setReadOnly(RegistrationModel::LoginNameField, true);

  if (user.password().empty())
    authModel_.reset();
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

  WPushButton *okButton =
    bindWidget("ok-button",
               cpp14::make_unique<WPushButton>(tr("Wt.WMessageBox.Ok")));
  WPushButton *cancelButton = 
    bindWidget("cancel-button",
               cpp14::make_unique<WPushButton>(tr("Wt.WMessageBox.Cancel")));

  if (authModel_) {
    authModel_->setValue(AuthModel::LoginNameField,
			 user.identity(Identity::LoginName));

    updateViewField(authModel_.get(), AuthModel::PasswordField);

    authModel_->configureThrottling(okButton);

    WLineEdit *password = resolve<WLineEdit *>(AuthModel::PasswordField);
    password->setFocus(true);
  }

  updateView(registrationModel_.get());

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
  cancelButton->clicked().connect(this, &UpdatePasswordWidget::cancel);
}

std::unique_ptr<WWidget> UpdatePasswordWidget
::createFormWidget(WFormModel::Field field)
{
  std::unique_ptr<WFormWidget> result;

  if (field == RegistrationModel::LoginNameField) {
    result.reset(new WLineEdit());
  } else if (field == AuthModel::PasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(EchoMode::Password);
    result.reset(p);
  } else if (field == RegistrationModel::ChoosePasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(EchoMode::Password);
    p->keyWentUp().connect(this, &UpdatePasswordWidget::checkPassword);
    p->changed().connect(this, &UpdatePasswordWidget::checkPassword);
    result.reset(p);
  } else if (field == RegistrationModel::RepeatPasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(EchoMode::Password);
    p->changed().connect(this, &UpdatePasswordWidget::checkPassword2);
    result.reset(p);
  }

  return std::move(result);
}

void UpdatePasswordWidget::checkPassword()
{
  updateModelField(registrationModel_.get(),
		   RegistrationModel::ChoosePasswordField);
  registrationModel_->validateField(RegistrationModel::ChoosePasswordField);
  updateViewField(registrationModel_.get(),
		  RegistrationModel::ChoosePasswordField);
}

void UpdatePasswordWidget::checkPassword2()
{
  updateModelField(registrationModel_.get(),
		   RegistrationModel::RepeatPasswordField);
  registrationModel_->validateField(RegistrationModel::RepeatPasswordField);
  updateViewField(registrationModel_.get(),
		  RegistrationModel::RepeatPasswordField);
}

bool UpdatePasswordWidget::validate()
{
  bool valid = true;

  if (authModel_) {
    updateModelField(authModel_.get(), AuthModel::PasswordField);

    if (!authModel_->validate()) {
      updateViewField(authModel_.get(), AuthModel::PasswordField);
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

    updated_.emit();
  }
}

void UpdatePasswordWidget::cancel()
{
  canceled_.emit();
}

  }
}
