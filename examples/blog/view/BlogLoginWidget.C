/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WLineEdit>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/Auth/PasswordAuth>
#include <Wt/Auth/RegistrationWidget>
#include "BlogLoginWidget.h"
#include "../model/BlogSession.h"
#include "../model/Token.h"
#include "../model/User.h"

using namespace Wt;

BlogLoginWidget::BlogLoginWidget(BlogSession& session,
				 const std::string& basePath,
				 WContainerWidget *parent)
  : AuthWidget(session.passwordAuth()->baseAuth(),
	       session.users(), session.login(), parent)
{
  setInline(true);

  addPasswordAuth(session.passwordAuth());
  addOAuth(session.oAuth());

  setInternalBasePath(basePath + "login");
}

WWidget *BlogLoginWidget::createLoginView()
{
  WWidget *result = AuthWidget::createLoginView();

  WTemplate *t = dynamic_cast<WTemplate *>(result->find(PasswordLoginWidget));
  t->setTemplateText(tr("blog-login"));

  WLineEdit *name = t->resolve<WLineEdit *>("name");
  name->setEmptyText("login");
  name->setToolTip("login");

  WLineEdit *password = t->resolve<WLineEdit *>("password");
  password->setEmptyText("password");
  password->setToolTip("password");
  password->enterPressed().connect(this, &BlogLoginWidget::attemptLogin);

  return result;
}

WWidget *BlogLoginWidget::createLoggedInView()
{
  WWidget *result = AuthWidget::createLoggedInView();

  WTemplate *t = dynamic_cast<WTemplate *>(result);

  WText *logout = new WText(tr("logout"));
  logout->setStyleClass("link");
  logout->clicked().connect(&login(), &Auth::Login::logout);
  t->bindWidget("logout", logout);

  return result;
}
  
WWidget *BlogLoginWidget::createRegistrationView(const Auth::Identity& identity)
{
  WWidget *r = Auth::AuthWidget::createRegistrationView(identity);
  Auth::RegistrationWidget *rr = dynamic_cast<Auth::RegistrationWidget *>(r);

  if (rr)
    rr->setIdentityPolicy(Auth::RegistrationWidget::UserChosen);

  return rr;
}

