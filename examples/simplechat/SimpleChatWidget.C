/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SimpleChatWidget.h"
#include "SimpleChatServer.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WInPlaceEdit.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WPushButton.h>
#include <Wt/WCheckBox.h>

#include <iostream>

SimpleChatWidget::SimpleChatWidget(SimpleChatServer& server)
  : WContainerWidget(),
    server_(server),
    loggedIn_(false),
    userList_(0),
    messageReceived_(nullptr)
{
  user_ = server_.suggestGuest();
  letLogin();
}

SimpleChatWidget::~SimpleChatWidget()
{
  messageReceived_.reset();
  logout();
}

void SimpleChatWidget::connect()
{
  if (server_.connect
      (this, std::bind(&SimpleChatWidget::processChatEvent, this, std::placeholders::_1)))
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

  auto vLayout = setLayout(Wt::cpp14::make_unique<Wt::WVBoxLayout>());

  auto hLayout_(Wt::cpp14::make_unique<Wt::WHBoxLayout>());
  auto hLayout = hLayout_.get();
  vLayout->addLayout(std::move(hLayout_), 0,
		     Wt::AlignmentFlag::Top | Wt::AlignmentFlag::Left);

  hLayout->addWidget(Wt::cpp14::make_unique<Wt::WLabel>("User name:"),
		     0, Wt::AlignmentFlag::Middle);

  userNameEdit_ = hLayout->addWidget(Wt::cpp14::make_unique<Wt::WLineEdit>(user_),
				     0, Wt::AlignmentFlag::Middle);
  userNameEdit_->setFocus();

  auto button = hLayout->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Login"),
				    0, Wt::AlignmentFlag::Middle);

  button->clicked().connect(this, &SimpleChatWidget::login);
  userNameEdit_->enterPressed().connect(this, &SimpleChatWidget::login);

  statusMsg_ = vLayout->addWidget(Wt::cpp14::make_unique<Wt::WText>());
  statusMsg_->setTextFormat(Wt::TextFormat::Plain);
}

void SimpleChatWidget::login()
{
  if (!loggedIn()) {
    Wt::WString name = userNameEdit_->text();

    if (!messageReceived_)
      messageReceived_ = Wt::cpp14::make_unique<Wt::WSound>("sounds/message_received.mp3");

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

void SimpleChatWidget::createLayout(std::unique_ptr<WWidget> messages, std::unique_ptr<WWidget> userList,
				    std::unique_ptr<WWidget> messageEdit,
				    std::unique_ptr<WWidget> sendButton, std::unique_ptr<WWidget> logoutButton)
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
  auto vLayout = Wt::cpp14::make_unique<Wt::WVBoxLayout>();

  // Create a horizontal layout for the messages | userslist.
  auto hLayout = Wt::cpp14::make_unique<Wt::WHBoxLayout>();

  // Choose JavaScript implementation explicitly to avoid log warning (needed for resizable layout)
  hLayout->setPreferredImplementation(Wt::LayoutImplementation::JavaScript);

  // Add widget to horizontal layout with stretch = 1
  messages->setStyleClass("chat-msgs");
  hLayout->addWidget(std::move(messages), 1);

  // Add another widget to horizontal layout with stretch = 0
  userList->setStyleClass("chat-users");
  hLayout->addWidget(std::move(userList));

  hLayout->setResizable(0, true);

  // Add nested layout to vertical layout with stretch = 1
  vLayout->addLayout(std::move(hLayout), 1);

  // Add widget to vertical layout with stretch = 0
  messageEdit->setStyleClass("chat-noedit");
  vLayout->addWidget(std::move(messageEdit));

  // Create a horizontal layout for the buttons.
  hLayout = Wt::cpp14::make_unique<Wt::WHBoxLayout>();

  // Add button to horizontal layout with stretch = 0
  hLayout->addWidget(std::move(sendButton));

  // Add button to horizontal layout with stretch = 0
  hLayout->addWidget(std::move(logoutButton));

  // Add nested layout to vertical layout with stretch = 0
  vLayout->addLayout(std::move(hLayout), 0, Wt::AlignmentFlag::Left);

  this->setLayout(std::move(vLayout));
}

bool SimpleChatWidget::loggedIn() const
{
  return loggedIn_;
}

void SimpleChatWidget::render(Wt::WFlags<Wt::RenderFlag> flags)
{
  if (flags.test(Wt::RenderFlag::Full)) {
    if (loggedIn()) {
      /* Handle a page refresh correctly */
      messageEdit_->setText(Wt::WString::Empty);
      doJavaScript("setTimeout(function() { "
		   + messages_->jsRef() + ".scrollTop += "
		   + messages_->jsRef() + ".scrollHeight;}, 0);");
    }
  }

  WContainerWidget::render(flags);
}

bool SimpleChatWidget::startChat(const Wt::WString& user)
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

    auto messagesPtr = Wt::cpp14::make_unique<WContainerWidget>();
    auto userListPtr = Wt::cpp14::make_unique<WContainerWidget>();
    auto messageEditPtr = Wt::cpp14::make_unique<Wt::WTextArea>();
    auto sendButtonPtr = Wt::cpp14::make_unique<Wt::WPushButton>("Send");
    auto logoutButtonPtr = Wt::cpp14::make_unique<Wt::WPushButton>("Logout");

    messages_ = messagesPtr.get();
    userList_ = userListPtr.get();
    messageEdit_ = messageEditPtr.get();
    sendButton_ = sendButtonPtr.get();
    Wt::Core::observing_ptr<Wt::WPushButton> logoutButton = logoutButtonPtr.get();

    messageEdit_->setRows(2);
    messageEdit_->setFocus();

    // Display scroll bars if contents overflows
    messages_->setOverflow(Wt::Overflow::Auto);
    userList_->setOverflow(Wt::Overflow::Auto);

    createLayout(std::move(messagesPtr), std::move(userListPtr),
                 std::move(messageEditPtr),
                 std::move(sendButtonPtr), std::move(logoutButtonPtr));

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

    /*
     * Set the connection monitor
     *
     * The connection monitor is a javascript monitor that will
     * nootify the given object by calling the onChange method to
     * inform of connection change (use of websockets, connection
     * online/offline) Here we just disable the TextEdit when we are
     * offline and enable it once we're back online
     */
    Wt::WApplication::instance()->setConnectionMonitor(
		"window.monitor={ "
		"'onChange':function(type, newV) {"
		  "var connected = window.monitor.status.connectionStatus != 0;"
		  "if(connected) {"
			+ messageEdit_->jsRef() + ".disabled=false;"
			+ messageEdit_->jsRef() + ".placeholder='';"
		  "} else { "
			+ messageEdit_->jsRef() + ".disabled=true;"
			+ messageEdit_->jsRef() + ".placeholder='connection lost';"
		  "}"
		"}"
		"}"
		);

    // Bind the C++ and JavaScript event handlers.
    if (sendButton_) {
      sendButton_->clicked().connect(this, &SimpleChatWidget::send);
      sendButton_->clicked().connect(clearInput_);
      sendButton_->clicked().connect((WWidget *)messageEdit_,
				     &WWidget::setFocus);
    }
    messageEdit_->enterPressed().connect(this, &SimpleChatWidget::send);
    messageEdit_->enterPressed().connect(clearInput_);
    messageEdit_->enterPressed().connect((WWidget *)messageEdit_,
					 &WWidget::setFocus);

    // Prevent the enter from generating a new line, which is its default
    // action
    messageEdit_->enterPressed().preventDefaultAction();

    if (logoutButton)
      logoutButton->clicked().connect(this, &SimpleChatWidget::logout);

    auto nameEdit = Wt::cpp14::make_unique<Wt::WInPlaceEdit>();
    nameEdit->addStyleClass("name-edit");
    nameEdit->setButtonsEnabled(false);
    nameEdit->setText(user_);
    nameEdit->valueChanged().connect(this, &SimpleChatWidget::changeName);

    Wt::WTemplate *joinMsg = messages_->addWidget(Wt::cpp14::make_unique<Wt::WTemplate>(tr("join-msg.template")));
    joinMsg->bindWidget("name", std::move(nameEdit));
    joinMsg->setStyleClass("chat-msg");

    updateUsers();
    
    return true;
  } else
    return false;
}

void SimpleChatWidget::changeName(const Wt::WString& name)
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
      Wt::WCheckBox *w = userList_->addWidget(Wt::cpp14::make_unique<Wt::WCheckBox>(escapeText(*i)));
      w->setInline(false);

      UserMap::const_iterator j = oldUsers.find(*i);
      if (j != oldUsers.end())
	w->setChecked(j->second);
      else
	w->setChecked(true);

      users_[*i] = w->isChecked();
      w->changed().connect(std::bind(&SimpleChatWidget::updateUser, this, w));

      if (*i == user_)
	w->setStyleClass("chat-self");
    }
  }
}

void SimpleChatWidget::newMessage()
{ }

void SimpleChatWidget::updateUser(Wt::WCheckBox *b)
{
  users_[b->text()] = b->isChecked();
}

void SimpleChatWidget::processChatEvent(const ChatEvent& event)
{
  Wt::WApplication *app = Wt::WApplication::instance();

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
    Wt::WText *w = messages_->addWidget(Wt::cpp14::make_unique<Wt::WText>());

    /*
     * If it fails, it is because the content wasn't valid XHTML
     */
    if (!w->setText(event.formattedHTML(user_, Wt::TextFormat::XHTML))) {
      w->setText(event.formattedHTML(user_, Wt::TextFormat::Plain));
      w->setTextFormat(Wt::TextFormat::XHTML);
    }

    w->setInline(false);
    w->setStyleClass("chat-msg");

    /*
     * Leave no more than 100 messages in the back-log
     */
    if (messages_->count() > 100)
      messages_->removeChild(messages_->children()[0]);

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
