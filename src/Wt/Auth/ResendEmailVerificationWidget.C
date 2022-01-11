#include "ResendEmailVerificationWidget.h"

#include "Wt/Auth/AuthService.h"

#include "Wt/WApplication.h"
#include "Wt/WLineEdit.h"
#include "Wt/WPushButton.h"
#include "Wt/WTheme.h"

namespace Wt {
  namespace Auth {

ResendEmailVerificationWidget::ResendEmailVerificationWidget(const User& user,
                                                             const AuthService& auth)
  : WTemplate(tr("Wt.Auth.template.resend-email-verification")),
    user_(user),
    baseAuth_(auth)
{
  addFunction("id", &Functions::id);
  addFunction("tr", &Functions::tr);
  addFunction("block", &Functions::block);

  WLineEdit *email = bindWidget("email", std::make_unique<WLineEdit>());
  email->setFocus(true);
  bindEmpty("email-info");
  
  WPushButton *okButton = bindWidget
    ("send-button",
     std::make_unique<WPushButton>(tr("Wt.Auth.send")));

  WPushButton *cancelButton = bindWidget
    ("cancel-button",
     std::make_unique<WPushButton>(tr("Wt.WMessageBox.Cancel")));

  okButton->clicked().connect(this, &ResendEmailVerificationWidget::send);
  cancelButton->clicked().connect(this, &ResendEmailVerificationWidget::cancel);
}

void ResendEmailVerificationWidget::send()
{
  WFormWidget *email = resolve<WFormWidget *>("email");
  std::string emailValue = email->valueText().toUTF8();
  bool emailMatches = emailValue == user_.unverifiedEmail();

  if (emailMatches) {
    baseAuth_.verifyEmailAddress(user_, emailValue);
    removeFromParent();
  } else {
    bindString("email-info", tr("Wt.Auth.resend-email-error"));
    WValidator::Result validation(ValidationState::Invalid, tr("Wt.Auth.resend-email-error"));
    WApplication::instance()->theme()
      ->applyValidationStyle(email, validation, ValidationAllStyles);
  }
}

void ResendEmailVerificationWidget::cancel()
{
  removeFromParent();
}

  }
}
