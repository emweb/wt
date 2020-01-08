// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef POPUP_CHATWIDGET_H_
#define POPUP_CHATWIDGET_H_

#include "SimpleChatWidget.h"

/**
 * \defgroup chatexample Chat example
 */
/*@{*/

/*! \brief A popup chat widget.
 */
class PopupChatWidget : public SimpleChatWidget
{
public:
  PopupChatWidget(SimpleChatServer& server, const std::string& id);

  void setName(const Wt::WString& name);

protected:
  virtual void createLayout(std::unique_ptr<WWidget> messages, std::unique_ptr<WWidget> userList,
                            std::unique_ptr<WWidget> messageEdit,
                            std::unique_ptr<WWidget> sendButton, std::unique_ptr<WWidget> logoutButton);

  virtual void updateUsers();
  virtual void newMessage();

private:
  Wt::WString   name_;
  Wt::WText    *title_;
  Wt::WWidget  *bar_;
  bool      online_, minimized_;
  int       missedMessages_;

  void toggleSize();
  void goOnline();
  bool minimized() const;

  std::unique_ptr<Wt::WContainerWidget> createBar();
};

/*@}*/

#endif // POPUP_CHATWIDGET_H_
