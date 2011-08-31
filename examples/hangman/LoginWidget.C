/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WText>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTemplate>
#include <Wt/WContainerWidget>
#include <Wt/WComboBox>
#include <Wt/Dbo/Dbo>

#include "LoginWidget.h"
#include "HangmanApplication.h"

using namespace Wt;
using namespace Wt::Dbo;

LoginWidget::LoginWidget(WContainerWidget *parent):
   WCompositeWidget(parent)
{
  setImplementation(impl_ = new WTemplate(tr("login.form")));
  
  impl_->bindString("intro", tr("login.intro"));
  
  userName_ = new WLineEdit();
  impl_->bindWidget("username", userName_);
  userName_->setId("username");

  passWord_ = new WLineEdit();
  impl_->bindWidget("password", passWord_);
  passWord_->setEchoMode(WLineEdit::Password);
  passWord_->setId("password");

  language_ = new WComboBox();
  impl_->bindWidget("language", language_);
  language_->insertItem(0, "English words (18957 words)");
  language_->insertItem(1, "Nederlandse woordjes (1688 woorden)");
  language_->setId("language");

  WPushButton *loginButton = new WPushButton("Login");
  impl_->bindWidget("login", loginButton);
  loginButton->clicked().connect(this, &LoginWidget::checkCredentials);

  impl_->bindString("login-error", "");
}

void LoginWidget::checkCredentials()
{
  std::string userName = userName_->text().toUTF8();
  std::string passWord = passWord_->text().toUTF8();

  HangmanApplication *app = HangmanApplication::instance();

  Session& session = app->session;
  Transaction transaction(session);
  ptr<User> user 
    = session.find<User>().where("name = ?").bind(userName);
  if (!user)
    user = session.add(new User(userName, passWord));
  
  //TODO currently cleartext -> will be changed to use the Wt auth module
  if (user->password != passWord) {
    impl_->bindString("login-error", tr("login.error"));

    userName_->setText("");
    passWord_->setText("");
  } else {
    user.modify()->lastLogin = WDateTime::currentDateTime();
    app->user = user;
    app->dictionary = (Dictionary) language_->currentIndex();
    
    app->setInternalPath("play", true);
   }

   transaction.commit();
}

