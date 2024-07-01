#include "myauthwidget.h"

#include "pinwidget.h"

#include "Wt/Auth/Mfa/AbstractMfaProcess.h"

MyAuthWidget::MyAuthWidget(const Wt::Auth::AuthService& baseAuth, Wt::Auth::AbstractUserDatabase& users, Wt::Auth::Login& login)
  : Wt::Auth::AuthWidget(baseAuth, users, login)
{
}

std::unique_ptr<Wt::Auth::Mfa::AbstractMfaProcess> MyAuthWidget::createMfaProcess()
{
  auto widget = std::make_unique<PinWidget>(*model()->baseAuth(), model()->users(), login());
  widget->authenticated().connect([this](Wt::Auth::Mfa::AuthenticationResult res) {
    authenticated_.emit(res);
  });
  return std::move(widget);
}

