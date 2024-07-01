#pragma once

#include "mysession.h"

#include "Wt/Http/Request.h"
#include "Wt/Http/Response.h"

#include "Wt/WResource.h"

class PhoneWidget;

class QrCodeHandler final : public Wt::WResource
{
public:
  QrCodeHandler(MySession& session, PhoneWidget* phoneWidget);

  void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) final;

  Wt::Signal<>& allowSignin() { return allowSignin_; }

private:
  MySession& session_;
  PhoneWidget* phoneWidget_ = nullptr;

  Wt::Signal<> allowSignin_;
};
