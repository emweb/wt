// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SIMPLECHATWIDGET_H_
#define SIMPLECHATWIDGET_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WSound.h>

#include "SimpleChatServer.h"

class ChatEvent;

/**
 * \defgroup chatexample Chat example
 */
/*@{*/

/*! \brief A self-contained chat widget.
 */
class SimpleChatWidget : public Wt::WContainerWidget,
			 public SimpleChatServer::Client
{
public:
  /*! \brief Create a chat widget that will connect to the given server.
   */
  SimpleChatWidget(SimpleChatServer& server);

  /*! \brief Delete a chat widget.
   */
  ~SimpleChatWidget();

  void connect();
  void disconnect();


  /*! \brief Show a simple login screen.
   */
  void letLogin();

  /*! \brief Start a chat for the given user.
   *
   * Returns false if the user could not login.
   */
  bool startChat(const Wt::WString& user);

  void logout();

  SimpleChatServer& server() { return server_; }

  int userCount() { return users_.size(); }

  const Wt::WString& userName() const { return user_; }

protected:
  virtual void createLayout(std::unique_ptr<Wt::WWidget> messages, std::unique_ptr<Wt::WWidget> userList,
			    std::unique_ptr<Wt::WWidget> messageEdit,
			    std::unique_ptr<Wt::WWidget> sendButton, std::unique_ptr<Wt::WWidget> logoutButton);

  virtual void updateUsers();
  virtual void newMessage();

  virtual void render(Wt::WFlags<Wt::RenderFlag> flags);

protected:
  bool loggedIn() const;

private:
  typedef std::map<Wt::WString, bool> UserMap;
  UserMap users_;

  SimpleChatServer&         server_;
  bool                      loggedIn_;

  Wt::JSlot                     clearInput_;

  Wt::WString                   user_;

  Wt::WLineEdit                *userNameEdit_;
  Wt::WText                    *statusMsg_;

  Wt::WContainerWidget         *messages_;
  Wt::WTextArea                *messageEdit_;
  Wt::Core::observing_ptr<Wt::WPushButton> sendButton_;
  Wt::Core::observing_ptr<Wt::WContainerWidget> userList_;

  std::unique_ptr<Wt::WSound>   messageReceived_;

  void login();
  void changeName(const Wt::WString& name);
  void send();
  void updateUser(Wt::WCheckBox *b);

  /* called from another session */
  void processChatEvent(const ChatEvent& event);
};

/*@}*/

#endif // SIMPLECHATWIDGET
