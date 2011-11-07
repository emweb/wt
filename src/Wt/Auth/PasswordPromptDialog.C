/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/EnterPasswordFields"
#include "Wt/Auth/Login"
#include "Wt/Auth/AbstractPasswordAuth"
#include "Wt/Auth/PasswordPromptDialog"

#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WTemplate"
#include "Wt/WText"

namespace Wt {
  namespace Auth {

PasswordPromptDialog::PasswordPromptDialog(Login& login,
					   const AbstractPasswordAuth& auth)
  : WDialog(tr("Wt.Auth.enter-password")),
    login_(login),
    auth_(auth)
{
  impl_ = new WTemplate(tr("Wt.Auth.template.password-prompt"));
  impl_->bindFunction("id", &WTemplate::Functions::id);
  impl_->bindFunction("tr", &WTemplate::Functions::tr);

  WLineEdit *nameEdit = new WLineEdit(login.user().identity());
  nameEdit->disable();
  nameEdit->addStyleClass("Wt-disabled");

  passwordEdit_ = new WLineEdit();
  WText *passwdInfo = new WText();
  WPushButton *okButton = new WPushButton(tr("Wt.WMessageBox.Ok"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  enterPasswordFields_ = new EnterPasswordFields(auth, passwordEdit_,
						 passwdInfo, okButton, this);

  impl_->bindWidget("identity", nameEdit);
  impl_->bindWidget("password", passwordEdit_);
  impl_->bindWidget("password-info", passwdInfo);
  impl_->bindWidget("ok-button", okButton);
  impl_->bindWidget("cancel-button", cancelButton);

  okButton->clicked().connect(this, &PasswordPromptDialog::check);
  cancelButton->clicked().connect(this, &PasswordPromptDialog::reject);

  contents()->addWidget(impl_);
}

void PasswordPromptDialog::check()
{
  bool valid = enterPasswordFields_->validate(login_.user());

  if (valid) {
    Login *login = &login_;
    accept();
    login->login(login->user(), StrongLogin);
  }
}

  }
}
