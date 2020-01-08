/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "AuthWidget.h"
#include "RegistrationView.h"
#include "model/Session.h"
#include "model/UserDetailsModel.h"

AuthWidget::AuthWidget(Session& session)
  : Auth::AuthWidget(Session::auth(), session.users(), session.login()),
    session_(session)
{  }

std::unique_ptr<WWidget> AuthWidget::createRegistrationView(const Auth::Identity& id)
{
  auto registrationView = cpp14::make_unique<RegistrationView>(session_, this);
  std::unique_ptr<Auth::RegistrationModel> model = createRegistrationModel();

  if (id.isValid())
    model->registerIdentified(id);

  registrationView->setModel(std::move(model));
  return std::move(registrationView);
}
