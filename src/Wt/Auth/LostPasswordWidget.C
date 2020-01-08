/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/LostPasswordWidget.h"
#include "Wt/Auth/AuthService.h"

#include "Wt/WApplication.h"
#include "Wt/WLineEdit.h"
#include "Wt/WMessageBox.h"
#include "Wt/WPushButton.h"

namespace {
}

namespace Wt {
  namespace Auth {

LostPasswordWidget::LostPasswordWidget(AbstractUserDatabase& users,
				       const AuthService& auth)
  : WTemplate(tr("Wt.Auth.template.lost-password")),
    users_(users),
    baseAuth_(auth)
{
  addFunction("id", &Functions::id);
  addFunction("tr", &Functions::tr);
  addFunction("block", &Functions::block);

  WLineEdit *email = bindWidget("email", cpp14::make_unique<WLineEdit>());
  email->setFocus(true);

  WPushButton *okButton = bindWidget
    ("send-button",
     cpp14::make_unique<WPushButton>(tr("Wt.Auth.send")));

  WPushButton *cancelButton = bindWidget
    ("cancel-button",
     cpp14::make_unique<WPushButton>(tr("Wt.WMessageBox.Cancel")));

  okButton->clicked().connect(this, &LostPasswordWidget::send);
  cancelButton->clicked().connect(this, &LostPasswordWidget::cancel);
}

void LostPasswordWidget::send()
{
  WFormWidget *email = resolve<WFormWidget *>("email");

  baseAuth_.lostPassword(email->valueText().toUTF8(), users_);

  cancel();
  // AFTER THIS CANCEL "this" IS DELETED, I.E. NOT VALID ANYMORE!

  std::unique_ptr<WMessageBox> box
    (new WMessageBox(tr("Wt.Auth.lost-password"), tr("Wt.Auth.mail-sent"),
                     Icon::None, StandardButton::Ok));
  box->show();

  WMessageBox *const boxPtr = box.get();
#ifndef WT_TARGET_JAVA
  box->buttonClicked().connect(nullptr, std::bind(&LostPasswordWidget::deleteBox, boxPtr));
#else // WT_TARGET_JAVA
  box->buttonClicked().connect(nullptr, std::bind(&LostPasswordWidget::deleteBox, this, boxPtr));
#endif // WT_TARGET_JAVA
#ifndef WT_TARGET_JAVA
  WApplication::instance()->addChild(std::move(box));
#endif // WT_TARGET_JAVA
}

void LostPasswordWidget::cancel()
{
  removeFromParent();
}

void LostPasswordWidget::deleteBox(Wt::WMessageBox *box)
{
#ifndef WT_TARGET_JAVA
  Wt::WApplication::instance()->removeChild(box);
#else // WT_TARGET_JAVA
  delete box;
#endif // WT_TARGET_JAVA
}

  }
}
