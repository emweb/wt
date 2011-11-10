/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/EnterPasswordFields"
#include "Wt/Auth/Login"
#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/PasswordPromptDialog"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WTemplate"
#include "Wt/WText"

namespace Wt {
  namespace Auth {

PasswordPromptDialog::PasswordPromptDialog(Login& login,
					   const AbstractPasswordService& auth)
  : WDialog(tr("Wt.Auth.enter-password")),
    login_(login),
    auth_(auth)
{
  impl_ = new WTemplate(tr("Wt.Auth.template.password-prompt"));
  impl_->addFunction("id", &WTemplate::Functions::id);
  impl_->addFunction("tr", &WTemplate::Functions::tr);

  WLineEdit *nameEdit = new WLineEdit(login.user().identity("username"));
  nameEdit->disable();
  nameEdit->addStyleClass("Wt-disabled");

  passwordEdit_ = new WLineEdit();
  WText *passwdInfo = new WText();
  WPushButton *okButton = new WPushButton(tr("Wt.WMessageBox.Ok"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  enterPasswordFields_ = new EnterPasswordFields(auth, passwordEdit_,
						 passwdInfo, okButton, this);

  impl_->bindWidget("user-name", nameEdit);
  impl_->bindWidget("password", passwordEdit_);
  impl_->bindWidget("password-info", passwdInfo);
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
     setMargin("-21em", Left); // .Wt-form width
     setMargin("-200px", Top); // ???
  }
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
