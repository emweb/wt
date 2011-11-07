/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/ChoosePasswordFields"
#include "Wt/Auth/AbstractPasswordAuth"

#include "Wt/WLineEdit"
#include "Wt/WText"

namespace Wt {
  namespace Auth {

ChoosePasswordFields::ChoosePasswordFields(const AbstractPasswordAuth& auth,
					   WLineEdit *passwd,
					   WText *passwdInfo,
					   WLineEdit *passwd2,
					   WText *passwd2Info,
					   WObject *parent)
  : WObject(parent),
    auth_(auth),
    passwd_(passwd),
    passwd2_(passwd2),
    passwdInfo_(passwdInfo),
    passwd2Info_(passwd2Info)
{
  init();
}

void ChoosePasswordFields::init()
{
  passwd_->setEchoMode(WLineEdit::Password);
  passwd2_->setEchoMode(WLineEdit::Password);

  passwdInfo_->setText(auth_.validatePassword(WString::Empty));
  passwd2Info_->setText(WString::tr("Wt.Auth.password2-info"));

  passwd_->keyWentUp().connect(this, &ChoosePasswordFields::checkPassword);

  // FIXME this should be a WValidator really
  passwd2_->keyWentUp().connect
    ("function(o) {"
     """var i=" + passwd2Info_->jsRef() + ",o1=" + passwd_->jsRef() + ";"
     """if ($(o1).hasClass('Wt-valid')) {"
     ""  "if (o.value == o1.value) {"
     ""     "$(o).addClass('Wt-valid'); "
     ""      WT_CLASS ".setHtml(i," + WString::tr("Wt.Auth.password2-valid")
     .jsStringLiteral() + ");"
     ""  "} else {"
     ""     "$(o).removeClass('Wt-valid');"
     ""      WT_CLASS ".setHtml(i," + WString::tr("Wt.Auth.password2-info")
     .jsStringLiteral() + ");"
     ""  "}"
     """}"
     "}");
}

void ChoosePasswordFields::checkPassword()
{
  checkPassword(false);
}

bool ChoosePasswordFields::checkPassword(bool indicateError)
{

  WString pwd = passwd_->text();

  WString e = auth_.validatePassword(pwd);
  bool valid = e.empty();

  if (valid)
    passwdInfo_->setText(WString::tr("Wt.Auth.password-valid"));
  else
    passwdInfo_->setText(e);

  passwdInfo_->toggleStyleClass("Wt-error", !valid);

  if (valid)
    passwd_->removeStyleClass("Wt-invalid");
  else if (indicateError)
    passwd_->addStyleClass("Wt-invalid");

  passwd_->toggleStyleClass("Wt-valid", valid);

  return valid;
}

bool ChoosePasswordFields::checkPassword2(bool indicateError)
{
  bool valid = passwd_->valueText() == passwd2_->valueText();

  if (!valid)
    passwd2Info_->setText(WString::tr("Wt.Auth.passwords-dont-match"));
  else if (indicateError)
    passwd2Info_->setText(WString::tr("Wt.Auth.password2-valid"));

  passwd2Info_->toggleStyleClass("Wt-error", indicateError && !valid);

  passwd2_->toggleStyleClass("Wt-invalid", indicateError && !valid);
  passwd2_->toggleStyleClass("Wt-valid", indicateError && valid);

  return valid;
}

bool ChoosePasswordFields::validate()
{
  bool valid = true;

  if (!checkPassword(true)) {
    valid = false;
    checkPassword2(false);
  } else
    if (!checkPassword2(true))
      valid = false;

  return valid;
}

  }
}
