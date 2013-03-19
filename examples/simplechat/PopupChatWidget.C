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
//  - i18n

PopupChatWidget::PopupChatWidget(SimpleChatServer& server,
				 const std::string& id)
  : SimpleChatWidget(server),
    missedMessages_(0)
{
  setId(id);

  if (Wt::WApplication::instance()->environment().agentIsIE()) {
    if (Wt::WApplication::instance()->environment().agent()
	== Wt::WEnvironment::IE6)
      setPositionScheme(Wt::Absolute);
    else
      setPositionScheme(Wt::Fixed);
  }

  implementJavaScript
    (&PopupChatWidget::toggleSize,
     "{"
     """var s = $('#" + id + "');"
     """s.toggleClass('chat-maximized chat-minimized');"
     + Wt::WApplication::instance()->javaScriptClass()
     + ".layouts2.scheduleAdjust(true);"
     "}");

  online_ = false;
  minimized_ = true;
  setStyleClass("chat-widget chat-minimized");

  clear();
  addWidget(createBar());
  updateUsers();

  connect();
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

Wt::WContainerWidget *PopupChatWidget::createBar() 
{
  Wt::WContainerWidget *bar = new Wt::WContainerWidget();
  bar->setStyleClass("chat-bar");

  Wt::WText *toggleButton = new Wt::WText();
  toggleButton->setInline(false);
  toggleButton->setStyleClass("chat-minmax");
  bar->clicked().connect(this, &PopupChatWidget::toggleSize);
  bar->clicked().connect(this, &PopupChatWidget::goOnline);

  bar->addWidget(toggleButton);

  title_ = new Wt::WText(bar);

  bar_ = bar;

  return bar;
}

void PopupChatWidget::toggleSize()
{
  minimized_ = !minimized_;
}

void PopupChatWidget::goOnline()
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

  missedMessages_ = 0;
  bar_->removeStyleClass("alert");
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
  bar->setMinimumSize(Wt::WLength::Auto, 20);
  layout->addWidget(messages, 1);
  layout->addWidget(messageEdit);

  setLayout(layout);
}

void PopupChatWidget::updateUsers()
{
  SimpleChatWidget::updateUsers();

  int count = server().users().size();

  if (!loggedIn()) {
    if (count == 0)
      title_->setText("Thoughts? Ventilate.");
    else if (count == 1)
      title_->setText("Chat: 1 user online");
    else
      title_->setText(Wt::WString("Chat: {1} users online").arg(count));
  } else {
    title_->setText(Wt::WString("Chat: <span class=\"self\">{1}</span>"
				" <span class=\"online\">({2} user{3})</span>")
		    .arg(userName()).arg(count).arg(count == 1 ? "" : "s"));
  }
}

void PopupChatWidget::newMessage()
{
  if (loggedIn() && minimized()) {
    ++missedMessages_;
    if (missedMessages_ == 1) {
      bar_->addStyleClass("alert");
    }
  }
}

bool PopupChatWidget::minimized() const
{
  return minimized_;
}
