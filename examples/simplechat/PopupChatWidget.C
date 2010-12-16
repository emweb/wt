/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/WVBoxLayout>

#include "PopupChatWidget.h"
#include "SimpleChatServer.h"

// TODO:
//  - oher color for jwt ?
//  - i18n

PopupChatWidget::PopupChatWidget(SimpleChatServer& server)
  : SimpleChatWidget(server)
{
  if (Wt::WApplication::instance()->environment().agentIsIE()) {
    if (Wt::WApplication::instance()->environment().agent() == Wt::WEnvironment::IE6)
      setPositionScheme(Wt::Absolute);
    else
      setPositionScheme(Wt::Fixed);
  }

  online_ = false;

  minimize();
}

void PopupChatWidget::setName(const Wt::WString& name)
{
  if (name.empty())
    return;

  if (online_) {
    int tries = 1;
    Wt::WString n = name;
    while (!server().changeName(name_, n))
      n = name + boost::lexical_cast<std::string>(++tries);

    name_ = n;
  } else
    name_ = name;
}

void PopupChatWidget::minimize()
{
  if (!online_) {
    clear();
    addWidget(createBar());
    title_->setText("Thoughts? Ventilate.");
  }

  setStyleClass("chat-widget chat-minimized");
}

Wt::WContainerWidget *PopupChatWidget::createBar() 
{
  Wt::WContainerWidget *bar = new Wt::WContainerWidget();
  bar->setStyleClass("chat-bar");

  Wt::WText *toggleButton = new Wt::WText();
  toggleButton->setInline(false);
  toggleButton->setStyleClass("chat-minmax");
  bar->clicked().connect(this, &PopupChatWidget::toggleSize);

  bar->addWidget(toggleButton);

  title_ = new Wt::WText(bar);

  return bar;
}

void PopupChatWidget::toggleSize()
{
  if (styleClass() == "chat-widget chat-minimized")
    maximize();
  else
    minimize();
}

void PopupChatWidget::createLayout(Wt::WWidget *messages,
				   Wt::WWidget *userList,
				   Wt::WWidget *messageEdit,
				   Wt::WWidget *sendButton,
				   Wt::WWidget *logoutButton)
{
  Wt::WVBoxLayout *layout = new Wt::WVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  Wt::WContainerWidget *bar = createBar();

  layout->addWidget(bar);
  layout->addWidget(messages, 1);
  layout->addWidget(messageEdit);

  setLayout(layout);
}

void PopupChatWidget::updateUsers()
{
  SimpleChatWidget::updateUsers();

  int count = server().users().size();

  if (count == 1)
    title_->setText("Chat: 1 user online");
  else
    title_->setText("Chat: "
		    + boost::lexical_cast<std::string>(count) + " users online");
}

void PopupChatWidget::maximize()
{
  if (!online_) {
    online_ = true;

    int tries = 1;
    Wt::WString name = name_;
    if (name.empty())
      name = server().suggestGuest();

    while (!startChat(name)) {
      if (name_.empty())
	name = server().suggestGuest();
      else
	name = name_ + boost::lexical_cast<std::string>(++tries);
    }

    name_ = name;
  }

  setStyleClass("chat-widget chat-maximized");
}
