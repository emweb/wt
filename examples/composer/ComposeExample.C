/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WContainerWidget>
#include <Wt/WStringUtil>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "Composer.h"
#include "ComposeExample.h"
#include "Contact.h"

ComposeExample::ComposeExample(WContainerWidget *parent)
  : WContainerWidget(parent)
{
  composer_ = new Composer(this);

  std::vector<Contact> addressBook;
  addressBook.push_back(Contact(L"Koen Deforche",
				L"koen.deforche@gmail.com"));
  addressBook.push_back(Contact(L"Koen alias1",
				L"koen.alias1@yahoo.com"));
  addressBook.push_back(Contact(L"Koen alias2",
				L"koen.alias2@yahoo.com"));
  addressBook.push_back(Contact(L"Koen alias3",
				L"koen.alias3@yahoo.com"));
  addressBook.push_back(Contact(L"Bartje",
				L"jafar@hotmail.com"));
  composer_->setAddressBook(addressBook);

  std::vector<Contact> contacts;
  contacts.push_back(Contact(L"Koen Deforche", L"koen.deforche@gmail.com"));

  composer_->setTo(contacts);
  composer_->setSubject("That's cool! Want to start your own google?");

  composer_->send.connect(this, &ComposeExample::send);
  composer_->discard.connect(this, &ComposeExample::discard);

  details_ = new WContainerWidget(this);

  new WText(tr("example.info"), details_);
}

void ComposeExample::send()
{
  WContainerWidget *feedback = new WContainerWidget(this);
  feedback->setStyleClass(L"feedback");

  WContainerWidget *horiz = new WContainerWidget(feedback);
  new WText(L"<p>We could have, but did not send the following email:</p>",
	    horiz);

  std::vector<Contact> contacts = composer_->to();
  if (!contacts.empty())
    horiz = new WContainerWidget(feedback);
  for (unsigned i = 0; i < contacts.size(); ++i) {
    new WText(L"To: \"" + contacts[i].name + L"\" <"
	      + contacts[i].email + L">", PlainText, horiz);
    new WBreak(horiz);
  }

  contacts = composer_->cc();
  if (!contacts.empty())
    horiz = new WContainerWidget(feedback);
  for (unsigned i = 0; i < contacts.size(); ++i) {
    new WText(L"Cc: \"" + contacts[i].name + L"\" <"
	      + contacts[i].email + L">", PlainText, horiz);
    new WBreak(horiz);
  }
  
  contacts = composer_->bcc();
  if (!contacts.empty())
    horiz = new WContainerWidget(feedback);
  for (unsigned i = 0; i < contacts.size(); ++i) {
    new WText(L"Bcc: \"" + contacts[i].name + L"\" <"
	      + contacts[i].email + L">", PlainText, horiz);
    new WBreak(horiz);
  }

  horiz = new WContainerWidget(feedback);
  new WText("Subject: \"" + composer_->subject() + "\"", PlainText, horiz);

  std::vector<Attachment> attachments = composer_->attachments();
  if (!attachments.empty())
    horiz = new WContainerWidget(feedback);
  for (unsigned i = 0; i < attachments.size(); ++i) {
    new WText(L"Attachment: \""
	      + attachments[i].fileName
	      + L"\" (" + attachments[i].contentDescription
	      + L")", PlainText, horiz);

    unlink(attachments[i].spoolFileName.c_str());

    new WText(", was in spool file: "
	      + attachments[i].spoolFileName, horiz);
    new WBreak(horiz);
  }

  std::wstring message = composer_->message();

  horiz = new WContainerWidget(feedback);
  new WText("Message body: ", horiz);
  new WBreak(horiz);

  if (!message.empty()) {
    new WText(message, PlainText, horiz);
  } else
    new WText("<i>(empty)</i>", horiz);

  delete composer_;
  delete details_;

  wApp->quit();
}

void ComposeExample::discard()
{
  WContainerWidget *feedback = new WContainerWidget(this);
  feedback->setStyleClass("feedback");

  WContainerWidget *horiz = new WContainerWidget(feedback);
  new WText("<p>Wise decision! Everyone's mailbox is already full anyway.</p>",
	    horiz);

  delete composer_;
  delete details_;

  wApp->quit();
}

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);

  // The following assumes composer.xml is in the webserver working directory
  // (but does not need to be deployed within docroot):
  app->messageResourceBundle().use(WApplication::appRoot() + "composer");

  // The following assumes composer.css is deployed in the seb server at the
  // same location as the application:
  app->useStyleSheet("composer.css");

  app->setTitle("Composer example");

  app->root()->addWidget(new ComposeExample());

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

