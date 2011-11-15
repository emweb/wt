/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/EnterPasswordFields"
#include "Wt/Auth/Login"
#include "Wt/Auth/UpdatePasswordWidget"
#include "Wt/Auth/User"

#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WText"

namespace Wt {
  namespace Auth {

UpdatePasswordWidget::UpdatePasswordWidget(const User& user,
					   const AbstractPasswordService& auth,
					   Login& login,
					   bool promptPassword,
					   WContainerWidget *parent)
  : WTemplate(tr("Wt.Auth.template.update-password"), parent),
    user_(user),
    promptPassword_(promptPassword),
    validated_(false),
    enterPasswordFields_(0)
{
  addFunction("id", &WTemplate::Functions::id);
  addFunction("tr", &WTemplate::Functions::tr);

  WLineEdit *nameEdit = new WLineEdit(user.identity(Identity::LoginName));
  nameEdit->disable();
  nameEdit->addStyleClass("Wt-disabled");
  bindWidget("user-name", nameEdit);

  WPushButton *okButton = new WPushButton(tr("Wt.WMessageBox.Ok"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  if (promptPassword_) {
    setCondition("if:old-password", true);
    WLineEdit *oldPasswd = new WLineEdit();
    WText *oldPasswdInfo = new WText();

    enterPasswordFields_ = new EnterPasswordFields(auth,
						   oldPasswd, oldPasswdInfo,
						   okButton, this);

    oldPasswd->setFocus();

    bindWidget("old-password", oldPasswd);
    bindWidget("old-password-info", oldPasswdInfo);
  }

  WLineEdit *password = new WLineEdit();
  password->setEchoMode(WLineEdit::Password);
  password->keyWentUp().connect
    (boost::bind(&UpdatePasswordWidget::checkPassword, this));
  password->changed().connect
    (boost::bind(&UpdatePasswordWidget::checkPassword, this));

  WText *passwordInfo = new WText();

  WLineEdit *password2 = new WLineEdit();
  password2->setEchoMode(WLineEdit::Password);
  password2->changed().connect
    (boost::bind(&UpdatePasswordWidget::checkPassword2, this));

  WText *password2Info = new WText();

  bindWidget("choose-password", password);
  bindWidget("choose-password-info", passwordInfo);

  bindWidget("repeat-password", password2);
  bindWidget("repeat-password-info", password2Info);

  model_ = new RegistrationModel(auth.baseAuth(), *user.database(),
				 login, this);

  model_->addPasswordAuth(&auth);

  model_->setValue(RegistrationModel::LoginName,
		   user.identity(Identity::LoginName));
  model_->setValue(RegistrationModel::Email,
		   WT_USTRING::fromUTF8(user.email() + " "
					+ user.unverifiedEmail()));

  model_->validatePasswordsMatchJS(password, password2, password2Info);

  passwordInfo->setText(model_->validationResult
			(RegistrationModel::Password).message());
  password2Info->setText(model_->validationResult
			 (RegistrationModel::Password2).message());

  okButton->clicked().connect(this, &UpdatePasswordWidget::doUpdate);
  cancelButton->clicked().connect(this, &UpdatePasswordWidget::close);

  bindWidget("ok-button", okButton);
  bindWidget("cancel-button", cancelButton);

  if (!promptPassword_)
    password->setFocus();
}

void UpdatePasswordWidget::updateModel(const std::string& var,
				       RegistrationModel::Field field)
{
  WFormWidget *edit = resolve<WFormWidget *>(var);
  model_->setValue(field, edit->valueText());
}

void UpdatePasswordWidget::updateView(const std::string& var,
				      RegistrationModel::Field field)
{
  WFormWidget *edit = resolve<WFormWidget *>(var);
  WText *info = resolve<WText *>(var + "-info");
  
  const WValidator::Result& v = model_->validationResult(field);
  info->setText(v.message());

  switch (v.state()) {
  case WValidator::InvalidEmpty:
  case WValidator::Invalid:
    edit->removeStyleClass("Wt-valid");
    if (validated_)
      edit->addStyleClass("Wt-invalid");
    info->addStyleClass("Wt-error");

    break;
  case WValidator::Valid:
    edit->removeStyleClass("Wt-invalid");
    if (validated_)
      edit->addStyleClass("Wt-valid");
    info->removeStyleClass("Wt-error");
  }
}

void UpdatePasswordWidget::checkPassword()
{
  updateModel("choose-password", RegistrationModel::Password);
  model_->validate(RegistrationModel::Password);
  updateView("choose-password", RegistrationModel::Password);
}

void UpdatePasswordWidget::checkPassword2()
{
  updateModel("repeat-password", RegistrationModel::Password2);
  model_->validate(RegistrationModel::Password2);
  updateView("repeat-password", RegistrationModel::Password2);
}

bool UpdatePasswordWidget::validate()
{
  validated_ = true;

  bool valid = true;

  if (enterPasswordFields_)
    if (!enterPasswordFields_->validate(user_))
      valid = false;

  checkPassword();
  checkPassword2();

  if (!model_->valid()) 
    valid = false;

  return valid;
}

void UpdatePasswordWidget::doUpdate()
{
  if (validate()) {
    const WT_USTRING& password = model_->value(RegistrationModel::Password);
    model_->passwordAuth()->updatePassword(user_, password);

    model_->login().login(user_);

    close();
  }
}

void UpdatePasswordWidget::close()
{
  delete this;
}

  }
}
