/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/LostPasswordWidget"
#include "Wt/Auth/AuthService"

#include "Wt/WLineEdit"
#include "Wt/WMessageBox"
#include "Wt/WPushButton"

namespace Wt {
  namespace Auth {

LostPasswordWidget::LostPasswordWidget(AbstractUserDatabase& users,
				       const AuthService& auth,
				       WContainerWidget *parent)
  : WTemplate(tr("Wt.Auth.template.lost-password"), parent),
    users_(users),
    baseAuth_(auth)
{
  addFunction("id", &Functions::id);
  addFunction("tr", &Functions::tr);
  addFunction("block", &Functions::block);

  WLineEdit *email = new WLineEdit();
  email->setFocus(true);

  WPushButton *okButton = new WPushButton(tr("Wt.Auth.send"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  okButton->clicked().connect(this, &LostPasswordWidget::send);
  cancelButton->clicked().connect(this, &LostPasswordWidget::cancel);

  bindWidget("email", email);
  bindWidget("send-button", okButton);
  bindWidget("cancel-button", cancelButton);
}

void LostPasswordWidget::send()
{
  WFormWidget *email = resolve<WFormWidget *>("email");

  baseAuth_.lostPassword(email->valueText().toUTF8(), users_);

  cancel();

  WMessageBox *const box = new WMessageBox(tr("Wt.Auth.lost-password"),
				     tr("Wt.Auth.mail-sent"),
				     NoIcon, Ok);
#ifndef WT_TARGET_JAVA
  box->buttonClicked().connect
    (boost::bind(&LostPasswordWidget::deleteBox, box));
#else
  box->buttonClicked().connect
    (boost::bind(&LostPasswordWidget::deleteBox, this, box));
#endif
  box->show();
}

void LostPasswordWidget::deleteBox(WMessageBox *box)
{
  delete box;
}

void LostPasswordWidget::cancel()
{
  delete this;
}

  }
}
