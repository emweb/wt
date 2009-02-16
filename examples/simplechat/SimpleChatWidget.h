// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2007 Koen Deforche
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SIMPLECHATWIDGET_H_
#define SIMPLECHATWIDGET_H_

#include <Wt/WContainerWidget>

namespace Wt {
  class WApplication;
  class WPushButton;
  class WText;
  class WLineEdit;
  class WTextArea;
}

class SimpleChatServer;
class ChatEvent;

/**
 * \defgroup chatexample Chat example
 */
/*@{*/

/*! \brief A self-contained chat widget.
 */
class SimpleChatWidget : public Wt::WContainerWidget
{
public:
  /*! \brief Create a chat widget that will connect to the given server.
   */
  SimpleChatWidget(SimpleChatServer& server, Wt::WContainerWidget *parent = 0);

  /*! \brief Delete a chat widget.
   */
  ~SimpleChatWidget();

  /*! \brief Show a simple login screen.
   */
  void letLogin();

  /*! \brief Start a chat for the given user.
   *
   * Returns false if the user could not login.
   */
  bool startChat(const Wt::WString& user);

private:
  SimpleChatServer&     server_;
  Wt::WApplication     *app_;

  Wt::WString           user_;

  Wt::WLineEdit        *userNameEdit_;
  Wt::WText            *statusMsg_;

  Wt::WContainerWidget *messages_;
  Wt::WContainerWidget *messageEditArea_;
  Wt::WTextArea        *messageEdit_;
  Wt::WPushButton      *sendButton_;
  Wt::WContainerWidget *userList_;

  boost::signals::connection eventConnection_;

  void login();
  void logout();
  void send();
  void updateUsers();

  /* called from another session */
  void processChatEvent(const ChatEvent& event);

  void onEditBlur();
  void onEditFocus();
};

/*@}*/

#endif // SIMPLECHATWIDGET
