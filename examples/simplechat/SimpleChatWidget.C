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
#include <Wt/WInPlaceEdit>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WTemplate>
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
    loggedIn_(false),
    userList_(0),
    messageReceived_(0)
{
  user_ = server_.suggestGuest();
  letLogin();
}

SimpleChatWidget::~SimpleChatWidget()
{
  delete messageReceived_;
  logout();
}

void SimpleChatWidget::connect()
{
  if (server_.connect
      (this, boost::bind(&SimpleChatWidget::processChatEvent, this, _1)))
    Wt::WApplication::instance()->enableUpdates(true);
}

void SimpleChatWidget::disconnect()
{
  if (server_.disconnect(this))
    Wt::WApplication::instance()->enableUpdates(false);
}

void SimpleChatWidget::letLogin()
{
  clear();

  WVBoxLayout *vLayout = new WVBoxLayout();
  setLayout(vLayout);

  WHBoxLayout *hLayout = new WHBoxLayout();
  vLayout->addLayout(hLayout, 0, AlignTop | AlignLeft);

  hLayout->addWidget(new WLabel("User name:"), 0, AlignMiddle);
  hLayout->addWidget(userNameEdit_ = new WLineEdit(user_), 0, AlignMiddle);
  userNameEdit_->setFocus();

  WPushButton *b = new WPushButton("Login");
  hLayout->addWidget(b, 0, AlignMiddle);

  b->clicked().connect(this, &SimpleChatWidget::login);
  userNameEdit_->enterPressed().connect(this, &SimpleChatWidget::login);

  vLayout->addWidget(statusMsg_ = new WText());
  statusMsg_->setTextFormat(PlainText);
}

void SimpleChatWidget::login()
{
  if (!loggedIn()) {
    WString name = userNameEdit_->text();

    if (!messageReceived_)
      messageReceived_ = new WSound("sounds/message_received.mp3");

    if (!startChat(name))
      statusMsg_->setText("Sorry, name '" + escapeText(name) +
			  "' is already taken.");
  }
}

void SimpleChatWidget::logout()
{
  if (loggedIn()) {
    loggedIn_ = false;
    server_.logout(user_);
    disconnect();

    letLogin();
  }
}

void SimpleChatWidget::createLayout(WWidget *messages, WWidget *userList,
				    WWidget *messageEdit,
				    WWidget *sendButton, WWidget *logoutButton)
{
  /*
   * Create a vertical layout, which will hold 3 rows,
   * organized like this:
   *
   * WVBoxLayout
   * --------------------------------------------
   * | nested WHBoxLayout (vertical stretch=1)  |
   * |                              |           |
   * |  messages                    | userList  |
   * |   (horizontal stretch=1)     |           |
   * |                              |           |
   * --------------------------------------------
   * | message edit area                        |
   * --------------------------------------------
   * | WHBoxLayout                              |
   * | send | logout                            |
   * --------------------------------------------
   */
  WVBoxLayout *vLayout = new WVBoxLayout();

  // Create a horizontal layout for the messages | userslist.
  WHBoxLayout *hLayout = new WHBoxLayout();

  // Add widget to horizontal layout with stretch = 1
  hLayout->addWidget(messages, 1);
  messages->setStyleClass("chat-msgs");

    // Add another widget to horizontal layout with stretch = 0
  hLayout->addWidget(userList);
  userList->setStyleClass("chat-users");

  hLayout->setResizable(0, true);

  // Add nested layout to vertical layout with stretch = 1
  vLayout->addLayout(hLayout, 1);

  // Add widget to vertical layout with stretch = 0
  vLayout->addWidget(messageEdit);
  messageEdit->setStyleClass("chat-noedit");

  // Create a horizontal layout for the buttons.
  hLayout = new WHBoxLayout();

  // Add button to horizontal layout with stretch = 0
  hLayout->addWidget(sendButton);

  // Add button to horizontal layout with stretch = 0
  hLayout->addWidget(logoutButton);

  // Add nested layout to vertical layout with stretch = 0
  vLayout->addLayout(hLayout, 0, AlignLeft);

  setLayout(vLayout);
}

bool SimpleChatWidget::loggedIn() const
{
  return loggedIn_;
}

void SimpleChatWidget::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull) {
    if (loggedIn()) {
      /* Handle a page refresh correctly */
      messageEdit_->setText(WString::Empty);
      doJavaScript("setTimeout(function() { "
		   + messages_->jsRef() + ".scrollTop += "
		   + messages_->jsRef() + ".scrollHeight;}, 0);");
    }
  }

  WContainerWidget::render(flags);
}

bool SimpleChatWidget::startChat(const WString& user)
{
  /*
   * When logging in, we pass our processChatEvent method as the function that
   * is used to indicate a new chat event for this user.
   */
  if (server_.login(user)) {
    loggedIn_ = true;
    connect();

    user_ = user;    

    clear();
    userNameEdit_ = 0;

    messages_ = new WContainerWidget();
    userList_ = new WContainerWidget();
    messageEdit_ = new WTextArea();
    messageEdit_->setRows(2);
    messageEdit_->setFocus();

    // Display scroll bars if contents overflows
    messages_->setOverflow(WContainerWidget::OverflowAuto);
    userList_->setOverflow(WContainerWidget::OverflowAuto);

    sendButton_ = new WPushButton("Send");
    WPushButton *logoutButton = new WPushButton("Logout");

    createLayout(messages_, userList_, messageEdit_, sendButton_, logoutButton);

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
      ("function(o, e) { setTimeout(function() {"
       "" + messageEdit_->jsRef() + ".value='';"
       "}, 0); }");

    // Bind the C++ and JavaScript event handlers.
    sendButton_->clicked().connect(this, &SimpleChatWidget::send);
    messageEdit_->enterPressed().connect(this, &SimpleChatWidget::send);
    sendButton_->clicked().connect(clearInput_);
    messageEdit_->enterPressed().connect(clearInput_);
    sendButton_->clicked().connect((WWidget *)messageEdit_,
				   &WWidget::setFocus);
    messageEdit_->enterPressed().connect((WWidget *)messageEdit_,
					 &WWidget::setFocus);

    // Prevent the enter from generating a new line, which is its default
    // action
    messageEdit_->enterPressed().preventDefaultAction();

    logoutButton->clicked().connect(this, &SimpleChatWidget::logout);

    WInPlaceEdit *nameEdit = new WInPlaceEdit();
    nameEdit->addStyleClass("name-edit");
    nameEdit->setButtonsEnabled(false);
    nameEdit->setText(user_);
    nameEdit->valueChanged().connect(this, &SimpleChatWidget::changeName);

    WTemplate *joinMsg = new WTemplate(tr("join-msg.template"), messages_);
    joinMsg->bindWidget("name", nameEdit);
    joinMsg->setStyleClass("chat-msg");

    if (!userList_->parent()) {
      delete userList_;
      userList_ = 0;
    }

    if (!sendButton_->parent()) {
      delete sendButton_;
      sendButton_ = 0;
    }

    if (!logoutButton->parent())
      delete logoutButton;

    updateUsers();
    
    return true;
  } else
    return false;
}

void SimpleChatWidget::changeName(const WString& name)
{
  if (!name.empty()) {
    if (server_.changeName(user_, name))
      user_ = name;
  }
}

void SimpleChatWidget::send()
{
  if (!messageEdit_->text().empty())
    server_.sendMessage(user_, messageEdit_->text());
}

void SimpleChatWidget::updateUsers()
{
  if (userList_) {
    userList_->clear();

    SimpleChatServer::UserSet users = server_.users();

    UserMap oldUsers = users_;
    users_.clear();

    for (SimpleChatServer::UserSet::iterator i = users.begin();
	 i != users.end(); ++i) {
      WCheckBox *w = new WCheckBox(escapeText(*i), userList_);
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
}

void SimpleChatWidget::newMessage()
{ }

void SimpleChatWidget::updateUser()
{
  WCheckBox *b = dynamic_cast<WCheckBox *>(sender());
  users_[b->text()] = b->isChecked();
}

void SimpleChatWidget::processChatEvent(const ChatEvent& event)
{
  WApplication *app = WApplication::instance();

  /*
   * This is where the "server-push" happens. The chat server posts to this
   * event from other sessions, see SimpleChatServer::postChatEvent()
   */

  /*
   * Format and append the line to the conversation.
   *
   * This is also the step where the automatic XSS filtering will kick in:
   * - if another user tried to pass on some JavaScript, it is filtered away.
   * - if another user did not provide valid XHTML, the text is automatically
   *   interpreted as PlainText
   */

  /*
   * If it is not a plain message, also update the user list.
   */
  if (event.type() != ChatEvent::Message) {
    if (event.type() == ChatEvent::Rename && event.user() == user_)
      user_ = event.data();

    updateUsers();
  }

  /*
   * This is the server call: we (schedule to) propagate the updated UI to
   * the client.
   *
   * This schedules an update and returns immediately
   */
  app->triggerUpdate();

  newMessage();

  /*
   * Anything else doesn't matter if we are not logged in.
   */
  if (!loggedIn())
    return;

  bool display = event.type() != ChatEvent::Message
    || !userList_
    || (users_.find(event.user()) != users_.end() && users_[event.user()]);

  if (display) {
    WText *w = new WText(messages_);

    /*
     * If it fails, it is because the content wasn't valid XHTML
     */
    if (!w->setText(event.formattedHTML(user_, XHTMLText))) {
      w->setText(event.formattedHTML(user_, PlainText));
      w->setTextFormat(XHTMLText);
    }

    w->setInline(false);
    w->setStyleClass("chat-msg");

    /*
     * Leave no more than 100 messages in the back-log
     */
    if (messages_->count() > 100)
      delete messages_->children()[0];

    /*
     * Little javascript trick to make sure we scroll along with new content
     */
    app->doJavaScript(messages_->jsRef() + ".scrollTop += "
		       + messages_->jsRef() + ".scrollHeight;");

    /* If this message belongs to another user, play a received sound */
    if (event.user() != user_ && messageReceived_)
      messageReceived_->play();
  }
}
