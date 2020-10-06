/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/Identity.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/AuthModel.h"
#include "Wt/Auth/PasswordPromptDialog.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLineEdit.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplateFormView.h"

namespace Wt {
  namespace Auth {

PasswordPromptDialog
::PasswordPromptDialog(Login& login, const std::shared_ptr<AuthModel>& model)
  : WDialog(tr("Wt.Auth.enter-password")),
    login_(login),
    model_(model)
{
  impl_ = contents()->addWidget(std::make_unique<WTemplateFormView>
				(tr("Wt.Auth.template.password-prompt")));

  model_->reset();
  model_->setValue(AuthModel::LoginNameField,
		   login_.user().identity(Identity::LoginName));
  model_->setReadOnly(AuthModel::LoginNameField, true);

  std::unique_ptr<WLineEdit> nameEdit(new WLineEdit());
  impl_->bindWidget(AuthModel::LoginNameField, std::move(nameEdit));
  impl_->updateViewField(model_.get(), AuthModel::LoginNameField);

  std::unique_ptr<WLineEdit> passwordEdit(new WLineEdit());
  passwordEdit->setEchoMode(EchoMode::Password);
  passwordEdit->setFocus(true);
  impl_->bindWidget(AuthModel::PasswordField, std::move(passwordEdit));
  impl_->updateViewField(model_.get(), AuthModel::PasswordField);

  WPushButton *okButton =
    impl_->bindWidget("ok-button",
                      std::make_unique<WPushButton>(tr("Wt.WMessageBox.Ok")));
  WPushButton *cancelButton =
    impl_->bindWidget("cancel-button",
		      std::make_unique<WPushButton>
		      (tr("Wt.WMessageBox.Cancel")));

  model_->configureThrottling(okButton);

  okButton->clicked().connect(this, &PasswordPromptDialog::check);
  cancelButton->clicked().connect(this, &PasswordPromptDialog::reject);

  if (!WApplication::instance()->environment().ajax()) {
    /*
     * try to center it better, we need to set the half width and
     * height as negative margins.
     */
     setMargin(WLength("-21em"), Side::Left); // .Wt-form width
     setMargin(WLength("-200px"), Side::Top); // ???
  }
}

void PasswordPromptDialog::check()
{
  impl_->updateModelField(model_.get(), AuthModel::PasswordField);

  if (model_->validate()) {
    Login *login = &login_;
    accept();
    login->login(login->user(), LoginState::Strong);    
  } else {
    impl_->updateViewField(model_.get(), AuthModel::PasswordField);
    WPushButton *okButton = impl_->resolve<WPushButton *>("ok-button");
    model_->updateThrottling(okButton);
  }
}

  }
}
