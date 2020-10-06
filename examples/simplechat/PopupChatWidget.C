/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WImage.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

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
	== Wt::UserAgent::IE6)
      setPositionScheme(Wt::PositionScheme::Absolute);
    else
      setPositionScheme(Wt::PositionScheme::Fixed);
  }

  implementJavaScript
    (&PopupChatWidget::toggleSize,
     "{"
     """var s = $('#" + id + "');"
     """s.toggleClass('chat-maximized chat-minimized');"
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
      n = name + std::to_string(++tries);

    name_ = n;
  } else
    name_ = name;
}

std::unique_ptr<Wt::WContainerWidget> PopupChatWidget::createBar()
{
  auto bar(std::make_unique<Wt::WContainerWidget>());
  bar->setStyleClass("chat-bar");

  auto toggleButton(std::make_unique<Wt::WText>());
  toggleButton->setInline(false);
  toggleButton->setStyleClass("chat-minmax");
  bar->clicked().connect(this, &PopupChatWidget::toggleSize);
  bar->clicked().connect(this, &PopupChatWidget::goOnline);

  bar->addWidget(std::move(toggleButton));

  title_ = bar->addWidget(std::make_unique<Wt::WText>());

  bar_ = bar.get();

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
	name = name_ + std::to_string(++tries);
    }

    name_ = name;
  }

  missedMessages_ = 0;
  bar_->removeStyleClass("alert");
}

void PopupChatWidget::createLayout(std::unique_ptr<Wt::WWidget> messages,
				   std::unique_ptr<Wt::WWidget> userList,
				   std::unique_ptr<Wt::WWidget> messageEdit,
				   std::unique_ptr<Wt::WWidget> sendButton,
				   std::unique_ptr<Wt::WWidget> logoutButton)
{
  auto layout(std::make_unique<Wt::WVBoxLayout>());
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  auto bar = layout->addWidget(createBar());
  bar->setMinimumSize(Wt::WLength::Auto, 20);
  layout->addWidget(std::move(messages), 1);
  layout->addWidget(std::move(messageEdit));

  setLayout(std::move(layout));
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
