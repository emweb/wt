#pragma once

#include "mysession.h"

#include "Wt/Auth/AuthModel.h"

class MyAuthModel : public Wt::Auth::AuthModel
{
public:
  MyAuthModel(MySession& session);

protected:
  bool hasMfaStep(const Wt::Auth::User& user) const final;

private:
  MySession& session_;
};
