// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
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
  PopupChatWidget(SimpleChatServer& server);

  void setName(const Wt::WString& name);

protected:
  virtual void createLayout(Wt::WWidget *messages, Wt::WWidget *userList,
			    Wt::WWidget *messageEdit,
			    Wt::WWidget *sendButton, Wt::WWidget *logoutButton);

  virtual void updateUsers();

private:
  Wt::WString name_;
  Wt::WText *title_;
  bool online_;

  void toggleSize();
  void minimize();
  void maximize();

  Wt::WContainerWidget *createBar();
};

/*@}*/

#endif // POPUP_CHATWIDGET_H_
