/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordAuth"
#include "Wt/Auth/ChoosePasswordFields"
#include "Wt/Auth/EnterPasswordFields"
#include "Wt/Auth/UpdatePasswordWidget"
#include "Wt/Auth/User"

#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WText"

namespace Wt {
  namespace Auth {

UpdatePasswordWidget::UpdatePasswordWidget(const User& user,
					   const AbstractPasswordAuth& auth,
					   bool promptPassword,
					   WContainerWidget *parent)
  : WTemplate(tr("Wt.Auth.template.update-password"), parent),
    user_(user),
    passwordAuth_(auth),
    promptPassword_(promptPassword),
    choosePasswordFields_(0),
    enterPasswordFields_(0)
{
  bindFunction("id", &WTemplate::Functions::id);
  bindFunction("tr", &WTemplate::Functions::tr);

  WPushButton *okButton = new WPushButton(tr("Wt.WMessageBox.Ok"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  if (promptPassword_) {
    WLineEdit *oldPasswd = new WLineEdit();
    WText *oldPasswdInfo = new WText();

    enterPasswordFields_ = new EnterPasswordFields(passwordAuth_,
						   oldPasswd, oldPasswdInfo,
						   okButton, this);

    bindString("old-password-display", "");
    bindWidget("old-password", oldPasswd);
    bindWidget("old-password-info", oldPasswdInfo);
  } else {
    bindString("old-password-display", "none");
    bindEmpty("old-password");
    bindEmpty("old-password-info");
  }

  WLineEdit *passwd = new WLineEdit();
  WText *passwdInfo = new WText();

  WLineEdit *passwd2 = new WLineEdit();
  WText *passwd2Info = new WText();

  choosePasswordFields_
    = new ChoosePasswordFields(passwordAuth_, passwd, passwdInfo,
			       passwd2, passwd2Info, this);

  okButton->clicked().connect(this, &UpdatePasswordWidget::doUpdate);
  cancelButton->clicked().connect(this, &UpdatePasswordWidget::close);

  WLineEdit *nameEdit = new WLineEdit(user.identity());
  nameEdit->disable();
  nameEdit->addStyleClass("Wt-disabled");
  bindWidget("identity", nameEdit);

  bindWidget("password", passwd);
  bindWidget("password-info", passwdInfo);

  bindWidget("password2", passwd2);
  bindWidget("password2-info", passwd2Info);

  bindWidget("ok-button", okButton);
  bindWidget("cancel-button", cancelButton);
}

bool UpdatePasswordWidget::validate()
{
  bool valid = true;

  if (enterPasswordFields_)
    if (!enterPasswordFields_->validate(user_))
      valid = false;

  if (!choosePasswordFields_->validate())
    valid = false;

  return valid;
}

void UpdatePasswordWidget::doUpdate()
{
  if (validate()) {
    WFormWidget *password = resolve<WFormWidget *>("password");
    passwordAuth_.updatePassword(user_, password->valueText());

    close();
  }
}

void UpdatePasswordWidget::close()
{
  delete this;
}

  }
}
