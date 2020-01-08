/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/RegistrationWidget.h>
#include "BlogLoginWidget.h"
#include "../model/BlogSession.h"
#include "../model/Token.h"
#include "../model/User.h"

BlogLoginWidget::BlogLoginWidget(BlogSession& session,
                                 const std::string& basePath)
  : AuthWidget(session.login())
{
  setInline(true);

  auto model
    = Wt::cpp14::make_unique<Wt::Auth::AuthModel>(session.passwordAuth()->baseAuth(),
                          session.users());
  model->addPasswordAuth(session.passwordAuth());
  model->addOAuth(session.oAuth());

  setModel(std::move(model));

  setInternalBasePath(basePath + "login");
}

void BlogLoginWidget::createLoginView()
{
  AuthWidget::createLoginView();

  setTemplateText(tr("blog-login"));

  Wt::WLineEdit *userName = resolve<Wt::WLineEdit *>("user-name");
  userName->setPlaceholderText("login");
  userName->setToolTip("login");

  Wt::WLineEdit *password = resolve<Wt::WLineEdit *>("password");
  password->setPlaceholderText("password");
  password->setToolTip("password");
}

void BlogLoginWidget::createLoggedInView()
{
  AuthWidget::createLoggedInView();

  auto logout = Wt::cpp14::make_unique<Wt::WText>(tr("logout"));
  logout->setStyleClass("link");
  logout->clicked().connect(&login(), &Wt::Auth::Login::logout);
  bindWidget("logout", std::move(logout));
}
