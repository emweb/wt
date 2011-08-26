/*
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WText>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>
#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WComboBox>
#include <Wt/Dbo/Dbo>

#include "LoginWidget.h"
#include "Dictionary.h"
#include "User.h"
#include "HangmanApplication.h"

using namespace Wt::Dbo;

LoginWidget::LoginWidget(WContainerWidget *parent):
   WContainerWidget(parent)
{
   setPadding(100, Left | Right);

   WText *title = new WText("Login", this);
   title->decorationStyle().font().setSize(WFont::XLarge);

   IntroText =
      new WText("<p>Hangman keeps track of the best players. To recognise "
		"you, we ask you to log in. If you never logged in before, "
		"choose any name and password. If you don't want to be in "
                "our database for some reason, use the 'guest/guest' "
		"account.</p>"
		"<p>Warning: hangman contains some words and "
		"pictures that may offend really young players.</p>", this);

   WTable *layout = new WTable(this);
   WLabel *usernameLabel = new WLabel("User name: ", layout->elementAt(0, 0));
   layout->elementAt(0, 0)->resize(WLength(14, WLength::FontEx), WLength::Auto);
   Username = new WLineEdit(layout->elementAt(0, 1));
   usernameLabel->setBuddy(Username);

   WLabel *passwordLabel = new WLabel("Password: ", layout->elementAt(1, 0));
   Password = new WLineEdit(layout->elementAt(1, 1));
   Password->setEchoMode(WLineEdit::Password);
   passwordLabel->setBuddy(Password);

   WLabel *languageLabel = new WLabel("Language: ", layout->elementAt(2, 0));
   Language = new WComboBox(layout->elementAt(2, 1));
   Language->insertItem(0, "English words (18957 words)");
   Language->insertItem(1, "Nederlandse woordjes (1688 woorden)");
   languageLabel->setBuddy(Language);

   new WBreak(this);

   WPushButton *LoginButton = new WPushButton("Login", this);
   LoginButton->clicked().connect(this, &LoginWidget::checkCredentials);
}

void LoginWidget::checkCredentials()
{
  std::string userName = Username->text().toUTF8();
  std::string passWord = Password->text().toUTF8();

   Dict = (Dictionary) Language->currentIndex();
   
   Session& session = HangmanApplication::instance()->session;
   Transaction transaction(session);
   ptr<User> user 
     = session.find<User>().where("name = ?").bind(userName);
   if (!user) {
     user = session.add(new User(userName, passWord));
   }

   //TODO currently cleartext -> will be changed to use the Wt auth module
   if (user->password != passWord) {
      IntroText
	->setText("<p>You entered the wrong password, or the username "
		  "combination is already in use. If you are a returning "
		  "user, please try again. If you are a new user, please "
		  "try a different name.</p>");
      IntroText->decorationStyle().setForegroundColor(Wt::red);
      Username->setText("");
      Password->setText("");
   } else {
     user.modify()->lastLogin = WDateTime::currentDateTime();
     HangmanApplication::instance()->user = user;

     if (user->name == "guest")
       confirmLogin("<p>Welcome guest, good luck.</p>");
     else
       confirmLogin("<p>Welcome, " + user->name +
		    ". Good luck with your first game!</p>");
   }

   transaction.commit();
}

void LoginWidget::confirmLogin(const std::string text)
{
   clear();

   WText *title = new WText("Loging successful", this);
   title->decorationStyle().font().setSize(WFont::XLarge);

   new WText(text, this);

   WPushButton* start = new WPushButton("Start playing", this);

   start->clicked().connect(boost::bind(&LoginWidget::onStartClicked, this));
}

void LoginWidget::onStartClicked()
{
  startPlaying.emit(Dict);
}

