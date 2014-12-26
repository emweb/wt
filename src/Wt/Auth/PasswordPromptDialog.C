/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/Identity"
#include "Wt/Auth/Login"
#include "Wt/Auth/AuthModel"
#include "Wt/Auth/PasswordPromptDialog"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WTemplateFormView"

namespace Wt {
  namespace Auth {

PasswordPromptDialog::PasswordPromptDialog(Login& login, AuthModel *model)
  : WDialog(tr("Wt.Auth.enter-password")),
    login_(login),
    model_(model)
{
  impl_ = new WTemplateFormView(tr("Wt.Auth.template.password-prompt"));

  model_->setValue(AuthModel::LoginNameField,
		   login_.user().identity(Identity::LoginName));
  model_->setReadOnly(AuthModel::LoginNameField, true);

  WLineEdit *nameEdit = new WLineEdit();
  impl_->bindWidget(AuthModel::LoginNameField, nameEdit);
  impl_->updateViewField(model_, AuthModel::LoginNameField);

  WLineEdit *passwordEdit = new WLineEdit();
  passwordEdit->setEchoMode(WLineEdit::Password);
  passwordEdit->setFocus(true);
  impl_->bindWidget(AuthModel::PasswordField, passwordEdit);
  impl_->updateViewField(model_, AuthModel::PasswordField);

  WPushButton *okButton = new WPushButton(tr("Wt.WMessageBox.Ok"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  model_->configureThrottling(okButton);

  impl_->bindWidget("ok-button", okButton);
  impl_->bindWidget("cancel-button", cancelButton);

  okButton->clicked().connect(this, &PasswordPromptDialog::check);
  cancelButton->clicked().connect(this, &PasswordPromptDialog::reject);

  contents()->addWidget(impl_);

  if (!WApplication::instance()->environment().ajax()) {
    /*
     * try to center it better, we need to set the half width and
     * height as negative margins.
     */
     setMargin(WLength("-21em"), Left); // .Wt-form width
     setMargin(WLength("-200px"), Top); // ???
  }
}

void PasswordPromptDialog::check()
{
  impl_->updateModelField(model_, AuthModel::PasswordField);

  if (model_->validate()) {
    Login *login = &login_;
    accept();
    login->login(login->user(), StrongLogin);    
  } else {
    impl_->updateViewField(model_, AuthModel::PasswordField);
    WPushButton *okButton = impl_->resolve<WPushButton *>("ok-button");
    model_->updateThrottling(okButton);
  }
}

  }
}
