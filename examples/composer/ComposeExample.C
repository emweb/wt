/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStringUtil.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "Composer.h"
#include "ComposeExample.h"
#include "Contact.h"

ComposeExample::ComposeExample()
  : WContainerWidget()
{
  composer_ = this->addWidget(std::make_unique<Composer>());

  std::vector<Contact> addressBook;
  addressBook.push_back(Contact(U"Koen Deforche",
                                U"koen.deforche@gmail.com"));
  addressBook.push_back(Contact(U"Koen alias1",
                                U"koen.alias1@yahoo.com"));
  addressBook.push_back(Contact(U"Koen alias2",
                                U"koen.alias2@yahoo.com"));
  addressBook.push_back(Contact(U"Koen alias3",
                                U"koen.alias3@yahoo.com"));
  addressBook.push_back(Contact(U"Bartje",
                                U"jafar@hotmail.com"));
  composer_->setAddressBook(addressBook);

  std::vector<Contact> contacts;
  contacts.push_back(Contact(U"Koen Deforche", U"koen.deforche@gmail.com"));

  composer_->setTo(contacts);
  composer_->setSubject("That's cool! Want to start your own google?");

  composer_->send.connect(this, &ComposeExample::send);
  composer_->discard.connect(this, &ComposeExample::discard);

  details_ = this->addWidget(std::make_unique<WContainerWidget>());

  details_->addWidget(std::make_unique<WText>(tr("example.info")));
}

void ComposeExample::send()
{
  WContainerWidget *feedback = this->addWidget(std::make_unique<WContainerWidget>());
  feedback->setStyleClass(U"feedback");

  WContainerWidget *horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  horiz->addWidget(std::make_unique<WText>(U"<p>We could have, but did not send the following email:</p>"));

  std::vector<Contact> contacts = composer_->to();
  if (!contacts.empty())
    horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  for (unsigned i = 0; i < contacts.size(); ++i) {
    horiz->addWidget(std::make_unique<WText>(U"To: \"" + contacts[i].name + U"\" <"
              + contacts[i].email + U">", TextFormat::Plain));
    horiz->addWidget(std::make_unique<WBreak>());
  }

  contacts = composer_->cc();
  if (!contacts.empty())
    horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  for (unsigned i = 0; i < contacts.size(); ++i) {
    horiz->addWidget(std::make_unique<WText>(U"Cc: \"" + contacts[i].name + U"\" <"
              + contacts[i].email + U">", TextFormat::Plain));
    horiz->addWidget(std::make_unique<WBreak>());
  }
  
  contacts = composer_->bcc();
  if (!contacts.empty())
    horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  for (unsigned i = 0; i < contacts.size(); ++i) {
      horiz->addWidget(std::make_unique<WText>(U"Bcc: \"" + contacts[i].name + U"\" <"
                + contacts[i].email + U">", TextFormat::Plain));
      horiz->addWidget(std::make_unique<WBreak>());
  }

  horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  horiz->addWidget(std::make_unique<WText>("Subject: \"" + composer_->subject() + "\"", TextFormat::Plain));

  std::vector<Attachment> attachments = composer_->attachments();
  if (!attachments.empty())
    horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  for (unsigned i = 0; i < attachments.size(); ++i) {
    horiz->addWidget(std::make_unique<WText>(U"Attachment: \""
	      + attachments[i].fileName
              + U"\" (" + attachments[i].contentDescription
              + U")", TextFormat::Plain));

    unlink(attachments[i].spoolFileName.c_str());

    horiz->addWidget(std::make_unique<WText>(", was in spool file: "
              + attachments[i].spoolFileName));
    horiz->addWidget(std::make_unique<WBreak>());
  }

  std::u32string message = composer_->message();

  horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  horiz->addWidget(std::make_unique<WText>("Message body: "));
  horiz->addWidget(std::make_unique<WBreak>());

  if (!message.empty()) {
    horiz->addWidget(std::make_unique<WText>(message, TextFormat::Plain));
  } else
    horiz->addWidget(std::make_unique<WText>("<i>(empty)</i>"));

  removeWidget(composer_);
  composer_ = nullptr;
  removeWidget(details_);
  details_ = nullptr;

  wApp->quit();
}

void ComposeExample::discard()
{
  WContainerWidget *feedback = this->addWidget(std::make_unique<WContainerWidget>());
  feedback->setStyleClass("feedback");

  WContainerWidget *horiz = feedback->addWidget(std::make_unique<WContainerWidget>());
  horiz->addWidget(std::make_unique<WText>("<p>Wise decision! Everyone's mailbox is already full anyway.</p>"));

  removeWidget(composer_);
  composer_ = nullptr;
  removeWidget(details_);
  details_ = nullptr;

  wApp->quit();
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> app
      = std::make_unique<WApplication>(env);

  // The following assumes composer.xml is in the webserver working directory
  // (but does not need to be deployed within docroot):
  app->messageResourceBundle().use(WApplication::appRoot() + "composer");

  // The following assumes composer.css is deployed in the seb server at the
  // same location as the application:
  app->useStyleSheet("composer.css");

  app->setTitle("Composer example");

  app->root()->addWidget(std::make_unique<ComposeExample>());

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

