#pragma once

#include "mysession.h"
#include "qrcodehandler.h"

#include "Wt/Auth/Mfa/AbstractMfaProcess.h"

#include "Wt/WCheckBox.h"
#include "Wt/WDialog.h"
#include "Wt/WLineEdit.h"
#include "Wt/WTemplate.h"

class PhoneWidget final : public Wt::Auth::Mfa::AbstractMfaProcess
{
public:
  PhoneWidget(MySession& session);

  std::unique_ptr<Wt::WWidget> createSetupView() final;
  std::unique_ptr<Wt::WWidget> createInputView() final;

  void processEnvironment() final;

  void setUpUserIdentity();

  Wt::Signal<Wt::Auth::Mfa::AuthenticationResult>& authenticated() { return authenticated_; }

private:
  MySession& session_;

  std::unique_ptr<QrCodeHandler> qrHandler_;

  Wt::WTemplate* view_ = nullptr;

  bool doRememberMe_ = false;

  Wt::Signal<Wt::Auth::Mfa::AuthenticationResult> authenticated_;

  void createQRCode(bool isSetup);
  void createRememberMe();

  void createQRHandlerResource();
  std::unique_ptr<Wt::WTemplate> createBaseView(bool isSetup);
};
