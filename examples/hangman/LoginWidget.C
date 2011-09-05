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

#include "LoginWidget.h"
#include "Session.h"

using namespace Wt;

LoginWidget::LoginWidget(Session* session, WContainerWidget *parent):
  WCompositeWidget(parent),
  session_(session)
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
  Dictionary dictionary = (Dictionary) language_->currentIndex();

  if (session_->login(userName, passWord)) {
    impl_->bindString("login-error", tr("login.error"));
    
    userName_->setText("");
    passWord_->setText("");
  } else {
    session_->setDictionary(dictionary);

    loggedIn_.emit();
  }
}
