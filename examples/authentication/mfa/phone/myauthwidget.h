#pragma once

#include "mysession.h"

#include "Wt/Auth/Mfa/AbstractMfaProcess.h"

#include "Wt/Auth/AuthWidget.h"

class MyAuthWidget final : public Wt::Auth::AuthWidget
{
public:
  MyAuthWidget(MySession& session);

  std::unique_ptr<Wt::Auth::Mfa::AbstractMfaProcess> createMfaProcess() final;

  void createMfaView() final;

private:
  MySession& session_;
  std::unique_ptr<Wt::Auth::Mfa::AbstractMfaProcess> phoneWidget_;
};

