#include "myauthwidget.h"

#include "authentry.h"
#include "phonewidget.h"

#include "Wt/Dbo/Transaction.h"

#include "Wt/WApplication.h"

MyAuthWidget::MyAuthWidget(MySession& session)
  : Wt::Auth::AuthWidget(session.auth(), session.users(), session.login()),
    session_(session)
{
}

std::unique_ptr<Wt::Auth::Mfa::AbstractMfaProcess> MyAuthWidget::createMfaProcess()
{
  return std::make_unique<PhoneWidget>(session_);
}

void MyAuthWidget::createMfaView()
{
  phoneWidget_ = createMfaProcess();
  PhoneWidget* widget = dynamic_cast<PhoneWidget*>(phoneWidget_.get());

  const Wt::Auth::User user = session_.login().user();

  Wt::Dbo::Transaction t(session_);
  Wt::Dbo::ptr<AuthEntry> hasEntry = session_.find<AuthEntry>("where auth_info_id = ?").bind(user.id());

  auto dialog = addChild(std::make_unique<Wt::WDialog>("MFA Phone Verification"));

  if (!hasEntry) {
    Wt::log("info") << "Showing Phone Widget (MFA) setup view.";
    dialog->contents()->addWidget(widget->createSetupView());
    dialog->finished().connect([dialog] {
      dialog->parent()->removeChild(dialog);
    });
    dialog->show();
  } else {
    widget->processEnvironment();
    if (login().state() != Wt::Auth::LoginState::RequiresMfa) {
      return;
    }

    Wt::log("info") << "Showing Phone Widget (MFA) input view.";
    dialog->contents()->addWidget(widget->createInputView());
    dialog->finished().connect([dialog] {
      dialog->parent()->removeChild(dialog);
    });
    dialog->show();
  }

  widget->authenticated().connect([this, dialog](Wt::Auth::Mfa::AuthenticationResult result) {
    if (result.status() == Wt::Auth::Mfa::AuthenticationStatus::Success) {
      createLoggedInView();
      Wt::WApplication::instance()->triggerUpdate();
      dialog->accept();
    }
  });
}

