/*
 * Copyright (C) 2007 Koen Deforche
 *
 * See the LICENSE file for terms of use.
 */

#include "SimpleChatWidget.h"
#include "SimpleChatServer.h"

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WPushButton>

#include <iostream>

using namespace Wt;

SimpleChatWidget::SimpleChatWidget(SimpleChatServer& server,
				   Wt::WContainerWidget *parent)
  : WContainerWidget(parent),
    server_(server),
    app_(WApplication::instance())
{
  user_ = server_.suggestGuest();
  letLogin();

  app_->enableUpdates();
}

SimpleChatWidget::~SimpleChatWidget()
{
  logout();
}

void SimpleChatWidget::letLogin()
{
  clear();

  WVBoxLayout *vLayout = new WVBoxLayout();
  setLayout(vLayout, AlignLeft | AlignTop);

  WHBoxLayout *hLayout = new WHBoxLayout();
  vLayout->addLayout(hLayout);

  hLayout->addWidget(new WLabel("User name:"), 0, AlignMiddle);
  hLayout->addWidget(userNameEdit_ = new WLineEdit(user_), 0, AlignMiddle);
  userNameEdit_->setFocus();

  WPushButton *b = new WPushButton("Login");
  hLayout->addWidget(b, 0, AlignMiddle);
  hLayout->addStretch(1);

  b->clicked.connect(SLOT(this, SimpleChatWidget::login));
  userNameEdit_->enterPressed.connect(SLOT(this, SimpleChatWidget::login));

  vLayout->addWidget(statusMsg_ = new WText());
  statusMsg_->setFormatting(WText::PlainFormatting);
}

void SimpleChatWidget::login()
{
  WString name = WWebWidget::escapeText(userNameEdit_->text());

  if (!startChat(name))
    statusMsg_->setText("Sorry, name '" + name + "' is already taken.");
}

void SimpleChatWidget::logout()
{
  if (eventConnection_.connected()) {
    eventConnection_.disconnect(); // do not listen for more events
    server_.logout(user_);

    letLogin();
  }
}

bool SimpleChatWidget::startChat(const WString& user)
{
  if (server_.login(user)) {
    eventConnection_
      = server_.chatEvent.connect(SLOT(this,
				       SimpleChatWidget::processChatEvent));
    user_ = user;    

    clear();

    /*
     * Create a vertical layout, which will hold 3 rows,
     * organized like this:
     *
     * WVBoxLayout
     * --------------------------------------------
     * | nested WHBoxLayout (vertical stretch=1)  |
     * |                              |           |
     * |  messages                    | userslist |
     * |   (horizontal stretch=1)     |           |
     * |                              |           |
     * --------------------------------------------
     * | message edit area                        |
     * --------------------------------------------
     * | WHBoxLayout                              |
     * | send | logout |       stretch = 1        |
     * --------------------------------------------
     */
    WVBoxLayout *vLayout = new WVBoxLayout();

    // Create a horizontal layout for the messages | userslist.
    WHBoxLayout *hLayout = new WHBoxLayout();

    // Add widget to horizontal layout with stretch = 1
    hLayout->addWidget(messages_ = new WContainerWidget(), 1);
    messages_->setStyleClass("chat-msgs");
    // Display scroll bars if contents overflows
    messages_->setOverflow(WContainerWidget::OverflowAuto);

    // Add another widget to hirozontal layout with stretch = 0
    hLayout->addWidget(userList_ = new WContainerWidget());
    userList_->setStyleClass("chat-users");
    userList_->setOverflow(WContainerWidget::OverflowAuto);

    // Add nested layout to vertical layout with stretch = 1
    vLayout->addLayout(hLayout, 1);

    // Add widget to vertical layout with stretch = 0
    vLayout->addWidget(messageEdit_ = new WTextArea());
    messageEdit_->setStyleClass("chat-noedit");
    messageEdit_->setRows(2);
    messageEdit_->setFocus();

    // Create a horizontal layout for the buttons.
    hLayout = new WHBoxLayout();

    // Add button to horizontal layout with stretch = 0
    hLayout->addWidget(sendButton_ = new WPushButton("Send"));
    WPushButton *b;

    // Add button to horizontal layout with stretch = 0
    hLayout->addWidget(b = new WPushButton("Logout", this));

    // Add stretching spacer to horizontal layout
    hLayout->addStretch(1);

    // Add nested layout to vertical layout with stretch = 0
    vLayout->addLayout(hLayout);

    setLayout(vLayout);

    /*
     * Connect event handlers
     */
    sendButton_->clicked.connect(SLOT(sendButton_, WPushButton::disable));
    sendButton_->clicked.connect(SLOT(messageEdit_, WTextArea::disable));
    sendButton_->clicked.connect(SLOT(this, SimpleChatWidget::send));

    messageEdit_->enterPressed.connect(SLOT(sendButton_, WPushButton::disable));
    messageEdit_->enterPressed.connect(SLOT(messageEdit_, WTextArea::disable));
    messageEdit_->enterPressed.connect(SLOT(this, SimpleChatWidget::send));

    b->clicked.connect(SLOT(this, SimpleChatWidget::logout));

    WText *msg
      = new WText(false,
		  "<span class='chat-info'>You are joining the conversation as "
		  + user_ + "</span>", messages_);
    msg->setStyleClass("chat-msg");

    updateUsers();
    
    return true;
  } else
    return false;
}

void SimpleChatWidget::send()
{
  if (!messageEdit_->text().empty()) {
    server_.sendMessage(user_, messageEdit_->text());
    messageEdit_->setText("");
  }

  messageEdit_->enable();
  messageEdit_->setFocus();
  sendButton_->enable();
}

void SimpleChatWidget::updateUsers()
{
  userList_->clear();

  SimpleChatServer::UserSet users = server_.users();

  WString usersStr;

  for (SimpleChatServer::UserSet::iterator i = users.begin();
       i != users.end(); ++i) {
    if (*i == user_)
      usersStr += "<span class='chat-self'>" + *i + "</span><br />";
    else
      usersStr += *i + "<br />";
  }

  userList_->addWidget(new WText(false, usersStr));
}

void SimpleChatWidget::processChatEvent(const ChatEvent& event)
{
  /*
   * This is where the "server-push" happens. This method is called
   * when a new event or message needs to be notified to the user. In
   * general, it is called from another session.
   *
   * First, we take the lock to safely manipulate the UI outside of the
   * normal event loop.
   */

  WApplication::UpdateLock lock = app_->getUpdateLock();

  WText *w = new WText(false, event.formattedHTML(user_), messages_);
  w->setStyleClass("chat-msg");

  /* no more than 100 messages back-log */
  if (messages_->count() > 100)
    delete messages_->children()[0];

  if (event.type() != ChatEvent::Message)
    updateUsers();

  /*
   * little javascript trick to make sure we scroll along with new content
   */
  app_->doJavaScript(messages_->jsRef() + ".scrollTop += "
		     + messages_->jsRef() + ".scrollHeight;");

  app_->triggerUpdate();
}
