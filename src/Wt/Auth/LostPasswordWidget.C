/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/LostPasswordWidget"
#include "Wt/Auth/BaseAuth"

#include "Wt/WLineEdit"
#include "Wt/WMessageBox"
#include "Wt/WPushButton"
#include "Wt/WText"

namespace Wt {
  namespace Auth {

LostPasswordWidget::LostPasswordWidget(AbstractUserDatabase& users,
				       const BaseAuth& auth,
				       WContainerWidget *parent)
  : WTemplate(tr("Wt.Auth.template.lost-password"), parent),
    users_(users),
    baseAuth_(auth)
{
  WLineEdit *email = new WLineEdit();

  WPushButton *okButton = new WPushButton(tr("Wt.Auth.send"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.Auth.cancel"));

  okButton->clicked().connect(this, &LostPasswordWidget::send);
  cancelButton->clicked().connect(this, &LostPasswordWidget::cancel);

  bindWidget("email", email);
  bindWidget("ok-button", okButton);
  bindWidget("cancel-button", cancelButton);
}

void LostPasswordWidget::send()
{
  WFormWidget *email = resolve<WFormWidget *>("email");

  baseAuth_.lostPassword(email->valueText().toUTF8(), users_);

  cancel();

  WMessageBox *box = new WMessageBox(tr("Wt.Auth.lost-password"),
				     tr("Wt.Auth.mail-sent"),
				     NoIcon, Ok);
  box->buttonClicked().connect
    (boost::bind(&LostPasswordWidget::deleteBox, box));
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
