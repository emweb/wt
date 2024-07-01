#pragma once

#include "Wt/Auth/Mfa/AbstractMfaProcess.h"

#include "Wt/WCheckBox.h"
#include "Wt/WCompositeWidget.h"
#include "Wt/WLineEdit.h"
#include "Wt/WTemplate.h"

class PinWidget final : public Wt::Auth::Mfa::AbstractMfaProcess
{
public:
  const int NUMBER_OF_DIGITS = 5;

  PinWidget(const Wt::Auth::AuthService& authService, Wt::Auth::AbstractUserDatabase& users, Wt::Auth::Login& login);

  std::unique_ptr<Wt::WWidget> createSetupView() final;
  std::unique_ptr<Wt::WWidget> createInputView() final;

  Wt::Signal<Wt::Auth::Mfa::AuthenticationResult>& authenticated() { return authenticated_; }

protected:
  void createCodeGenerator();
  void createCodeInput();
  void createLoginButton();
  void createRememberMe();

  void checkCodeInput();

  void update();

private:
  Wt::WTemplate* view_ = nullptr;

  std::string currentCode_;

  Wt::WLineEdit* codeInput_;
  Wt::WCheckBox* rememberMeField_;

  Wt::Signal<Wt::Auth::Mfa::AuthenticationResult> authenticated_;

  std::unique_ptr<Wt::WTemplate> createBaseView(bool isSetup);
};
