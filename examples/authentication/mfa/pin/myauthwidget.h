#pragma once

#include "Wt/Auth/Mfa/AbstractMfaProcess.h"
#include "Wt/Auth/AuthWidget.h"

class MyAuthWidget final : public Wt::Auth::AuthWidget
{
public:
  MyAuthWidget(const Wt::Auth::AuthService& baseAuth, Wt::Auth::AbstractUserDatabase& users, Wt::Auth::Login& login);

  std::unique_ptr<Wt::Auth::Mfa::AbstractMfaProcess> createMfaProcess() final;

  Wt::Signal<Wt::Auth::Mfa::AuthenticationResult>& authenticated() { return authenticated_; }

private:
  Wt::Signal<Wt::Auth::Mfa::AuthenticationResult> authenticated_;
};

