/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SimpleChatWidget.h"
#include "SimpleChatServer.h"

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WPushButton>
#include <Wt/WCheckBox>

#include <iostream>

using namespace Wt;

SimpleChatWidget::SimpleChatWidget(SimpleChatServer& server,
				   Wt::WContainerWidget *parent)
  : WContainerWidget(parent),
    server_(server),
    app_(WApplication::instance()),
    messageReceived_("sounds/message_received.mp3")
{
  user_ = server_.suggestGuest();
  letLogin();

  // this widget supports server-side updates its processChatEvent()
  // method is connected to a slot that is triggered from outside this
  // session's event loop (usually because another user enters text).
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

  b->clicked().connect(this, &SimpleChatWidget::login);
  userNameEdit_->enterPressed().connect(this, &SimpleChatWidget::login);

  vLayout->addWidget(statusMsg_ = new WText());
  statusMsg_->setTextFormat(PlainText);
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
    // FIXME, chatEvent() needs to be protected by the server mutex too
    eventConnection_
      = server_.chatEvent().connect(this, &SimpleChatWidget::processChatEvent);
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

    hLayout->setResizable(0, true);

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
    hLayout->addWidget(b = new WPushButton("Logout"));

    // Add stretching spacer to horizontal layout
    hLayout->addStretch(1);

    // Add nested layout to vertical layout with stretch = 0
    vLayout->addLayout(hLayout);

    setLayout(vLayout);

    /*
     * Connect event handlers:
     *  - click on button
     *  - enter in text area
     *
     * We will clear the input field using a small custom client-side
     * JavaScript invocation.
     */

    // Create a JavaScript 'slot' (JSlot). The JavaScript slot always takes
    // 2 arguments: the originator of the event (in our case the
    // button or text area), and the JavaScript event object.
    clearInput_.setJavaScript
      ("function(o, e) {"
       "" + messageEdit_->jsRef() + ".value='';"
       "}");

    // Bind the C++ and JavaScript event handlers.
    sendButton_->clicked().connect(this, &SimpleChatWidget::send);
    messageEdit_->enterPressed().connect(this, &SimpleChatWidget::send);
    sendButton_->clicked().connect(clearInput_);
    messageEdit_->enterPressed().connect(clearInput_);
    sendButton_->clicked().connect(messageEdit_, &WLineEdit::setFocus);
    messageEdit_->enterPressed().connect(messageEdit_, &WLineEdit::setFocus);

    // Prevent the enter from generating a new line, which is its default
    // action
    messageEdit_->enterPressed().preventDefaultAction();

    b->clicked().connect(this, &SimpleChatWidget::logout);

    WText *msg = new WText
      ("<div><span class='chat-info'>You are joining the conversation as "
       + user_ + "</span></div>", messages_);
    msg->setStyleClass("chat-msg");

    updateUsers();
    
    return true;
  } else
    return false;
}

void SimpleChatWidget::send()
{
  if (!messageEdit_->text().empty())
    server_.sendMessage(user_, messageEdit_->text());
}

void SimpleChatWidget::updateUsers()
{
  userList_->clear();

  SimpleChatServer::UserSet users = server_.users();

  UserMap oldUsers = users_;
  users_.clear();

  for (SimpleChatServer::UserSet::iterator i = users.begin();
       i != users.end(); ++i) {
    WCheckBox *w = new WCheckBox(*i, userList_);
    w->setInline(false);

    UserMap::const_iterator j = oldUsers.find(*i);
    if (j != oldUsers.end())
      w->setChecked(j->second);
    else
      w->setChecked(true);

    users_[*i] = w->isChecked();
    w->changed().connect(this, &SimpleChatWidget::updateUser);

    if (*i == user_)
      w->setStyleClass("chat-self");
  }
}

void SimpleChatWidget::updateUser()
{
  WCheckBox *b = dynamic_cast<WCheckBox *>(sender());
  users_[b->text()] = b->isChecked();
}

void SimpleChatWidget::processChatEvent(const ChatEvent& event)
{
  /*
   * This is where the "server-push" happens. This method is called
   * when a new event or message needs to be notified to the user. In
   * general, it is called from another session.
   */

  /*
   * First, take the lock to safely manipulate the UI outside of the
   * normal event loop, by having exclusive access to the session.
   */
  WApplication::UpdateLock lock(app_);

  if (lock) {
    /*
     * Format and append the line to the conversation.
     *
     * This is also the step where the automatic XSS filtering will kick in:
     * - if another user tried to pass on some JavaScript, it is filtered away.
     * - if another user did not provide valid XHTML, the text is automatically
     *   interpreted as PlainText
     */
    bool needPush = false;

    /*
     * If it is not a normal message, also update the user list.
     */
    if (event.type() != ChatEvent::Message) {
      needPush = true;
      updateUsers();
    }

    bool display = event.type() != ChatEvent::Message
      || (users_.find(event.user()) != users_.end() && users_[event.user()]);

    if (display) {
      needPush = true;

      WText *w = new WText(event.formattedHTML(user_), messages_);
      w->setInline(false);
      w->setStyleClass("chat-msg");

      /*
       * Leave not more than 100 messages in the back-log
       */
      if (messages_->count() > 100)
	delete messages_->children()[0];

      /*
       * Little javascript trick to make sure we scroll along with new content
       */
      app_->doJavaScript(messages_->jsRef() + ".scrollTop += "
			 + messages_->jsRef() + ".scrollHeight;");

      /* If this message belongs to another user, play a received sound */
      if (event.user() != user_)
	messageReceived_.play();
    }

    if (needPush)
      app_->triggerUpdate();
  }
}
