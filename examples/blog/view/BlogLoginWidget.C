/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WLineEdit>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/RegistrationWidget>
#include "BlogLoginWidget.h"
#include "../model/BlogSession.h"
#include "../model/Token.h"
#include "../model/User.h"

using namespace Wt;

BlogLoginWidget::BlogLoginWidget(BlogSession& session,
				 const std::string& basePath,
				 WContainerWidget *parent)
  : AuthWidget(session.login(), parent)
{
  setInline(true);

  Auth::AuthModel *model
    = new Auth::AuthModel(session.passwordAuth()->baseAuth(),
			  session.users(), this);
  model->addPasswordAuth(session.passwordAuth());
  model->addOAuth(session.oAuth());

  setModel(model);

  setInternalBasePath(basePath + "login");
}

void BlogLoginWidget::createLoginView()
{
  AuthWidget::createLoginView();

  setTemplateText(tr("blog-login"));

  WLineEdit *userName = resolve<WLineEdit *>("user-name");
  userName->setEmptyText("login");
  userName->setToolTip("login");

  WLineEdit *password = resolve<WLineEdit *>("password");
  password->setEmptyText("password");
  password->setToolTip("password");
  password->enterPressed().connect(this,
				   &BlogLoginWidget::attemptPasswordLogin);
}

void BlogLoginWidget::createLoggedInView()
{
  AuthWidget::createLoggedInView();

  WText *logout = new WText(tr("logout"));
  logout->setStyleClass("link");
  logout->clicked().connect(&login(), &Auth::Login::logout);
  bindWidget("logout", logout);
}
