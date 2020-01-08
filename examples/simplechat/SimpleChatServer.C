/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SimpleChatServer.h"
#include <Wt/WServer.h>

#include <iostream>


const Wt::WString ChatEvent::formattedHTML(const Wt::WString& user,
				       Wt::TextFormat format) const
{
  switch (type_) {
  case Login:
    return Wt::WString("<span class='chat-info'>")
      + Wt::WWebWidget::escapeText(user_) + " joined.</span>";
  case Logout:
    return Wt::WString("<span class='chat-info'>")
      + ((user == user_) ?
	 Wt::WString("You") :
	 Wt::WWebWidget::escapeText(user_))
      + " logged out.</span>";
  case Rename:
    return "<span class='chat-info'>"
      + ((user == data_ || user == user_) ?
	 "You are" :
	 (Wt::WWebWidget::escapeText(user_) + " is"))
      + " now known as " + Wt::WWebWidget::escapeText(data_) + ".</span>";
  case Message:{
    Wt::WString result;

    result = Wt::WString("<span class='")
      + ((user == user_) ?
	 "chat-self" :
	 "chat-user")
      + "'>" + Wt::WWebWidget::escapeText(user_) + ":</span>";

    Wt::WString msg
      = (format == Wt::TextFormat::XHTML ? message_ : Wt::WWebWidget::escapeText(message_));

    if (message_.toUTF8().find(user.toUTF8()) != std::string::npos)
      return result + "<span class='chat-highlight'>" + msg + "</span>";
    else
      return result + msg;
  }
  default:
    return "";
  }
}


SimpleChatServer::SimpleChatServer(Wt::WServer& server)
  : server_(server)
{ }

bool SimpleChatServer::connect(Client *client,
			       const ChatEventCallback& handleEvent)
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);

  if (clients_.count(client) == 0) {
    ClientInfo clientInfo;
  
    clientInfo.sessionId = Wt::WApplication::instance()->sessionId();
    clientInfo.eventCallback = handleEvent;

    clients_[client] = clientInfo;

    return true;
  } else
    return false;
}

bool SimpleChatServer::disconnect(Client *client)
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);

  return clients_.erase(client) == 1;
}

bool SimpleChatServer::login(const Wt::WString& user)
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  
  if (users_.find(user) == users_.end()) {
    users_.insert(user);

    postChatEvent(ChatEvent(ChatEvent::Login, user));

    return true;
  } else
    return false;
}

void SimpleChatServer::logout(const Wt::WString& user)
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);

  UserSet::iterator i = users_.find(user);

  if (i != users_.end()) {
    users_.erase(i);

    postChatEvent(ChatEvent(ChatEvent::Logout, user));
  }
}

bool SimpleChatServer::changeName(const Wt::WString& user, const Wt::WString& newUser)
{
  if (user == newUser)
    return true;

  std::unique_lock<std::recursive_mutex> lock(mutex_);
  
  UserSet::iterator i = users_.find(user);

  if (i != users_.end()) {
    if (users_.count(newUser) == 0) {
      users_.erase(i);
      users_.insert(newUser);

      postChatEvent(ChatEvent(ChatEvent::Rename, user, newUser));

      return true;
    } else
      return false;
  } else
    return false;
}

Wt::WString SimpleChatServer::suggestGuest()
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);

  for (int i = 1;; ++i) {
    std::string s = "guest " + std::to_string(i);
    Wt::WString ss = s;

    if (users_.find(ss) == users_.end())
      return ss;
  }
}

void SimpleChatServer::sendMessage(const Wt::WString& user, const Wt::WString& message)
{
  postChatEvent(ChatEvent(user, message));
}

void SimpleChatServer::postChatEvent(const ChatEvent& event)
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);

  Wt::WApplication *app = Wt::WApplication::instance();

  for (ClientMap::const_iterator i = clients_.begin(); i != clients_.end();
       ++i) {
    /*
     * If the user corresponds to the current application, we directly
     * call the call back method. This avoids an unnecessary delay for
     * the update to the user causing the event.
     *
     * For other uses, we post it to their session. By posting the
     * event, we avoid dead-lock scenarios, race conditions, and
     * delivering the event to a session that is just about to be
     * terminated.
     */
    if (app && app->sessionId() == i->second.sessionId)
      i->second.eventCallback(event);
    else
      server_.post(i->second.sessionId,
                   std::bind(i->second.eventCallback, event));
  }
}

SimpleChatServer::UserSet SimpleChatServer::users()
{
  std::unique_lock<std::recursive_mutex> lock(mutex_);

  UserSet result = users_;

  return result;
}

