/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SimpleChatServer.h"
#include <Wt/WServer>

#include <iostream>
#include <boost/lexical_cast.hpp>

using namespace Wt;

const WString ChatEvent::formattedHTML(const WString& user) const
{
  switch (type_) {
  case Login:
    return "<span class='chat-info'>" + user_ + " joined.</span>";
  case Logout:
    return "<span class='chat-info'>"
      + ((user == user_) ? "You" : user_)
      + " logged out.</span>";
  case Rename:
    return "<span class='chat-info'>"
      + ((user == data_ || user == user_) ?
	 "You are" : (user_ + " is")) + " now known as "
      + data_ + ".</span>";
  case Message:{
    WString result;

    result = WString("<span class='")
      + ((user == user_) ? "chat-self" : "chat-user")
      + "'>" + user_ + ":</span>";

    if (message_.toUTF8().find(user.toUTF8()) != std::string::npos)
      return result + "<span class='chat-highlight'>" + message_ + "</span>";
    else
      return result + message_;
  }
  default:
    return "";
  }
}


SimpleChatServer::SimpleChatServer(WServer& server)
  : server_(server)
{ }

bool SimpleChatServer::login(const WString& user,
			     const ChatEventCallback& handleEvent)
{
  boost::recursive_mutex::scoped_lock lock(mutex_);
  
  if (users_.find(user) == users_.end()) {
    UserInfo userInfo;
    userInfo.sessionId = WApplication::instance()->sessionId();
    userInfo.eventCallback = handleEvent;

    users_[user] = userInfo;

    postChatEvent(ChatEvent(ChatEvent::Login, user));

    return true;
  } else
    return false;
}

void SimpleChatServer::logout(const WString& user)
{
  boost::recursive_mutex::scoped_lock lock(mutex_);

  UserMap::iterator i = users_.find(user);

  if (i != users_.end()) {
    users_.erase(i);

    postChatEvent(ChatEvent(ChatEvent::Logout, user));
  }
}

bool SimpleChatServer::changeName(const WString& user, const WString& newUser)
{
  if (user == newUser)
    return true;

  boost::recursive_mutex::scoped_lock lock(mutex_);
  
  UserMap::iterator i = users_.find(user);

  if (i != users_.end()) {
    if (users_.find(newUser) == users_.end()) {
      UserInfo info = i->second;
      users_.erase(i);
      users_[newUser] = info;

      postChatEvent(ChatEvent(ChatEvent::Rename, user, newUser));

      return true;
    } else
      return false;
  } else
    return false;
}

WString SimpleChatServer::suggestGuest()
{
  boost::recursive_mutex::scoped_lock lock(mutex_);

  for (int i = 1;; ++i) {
    std::string s = "guest " + boost::lexical_cast<std::string>(i);
    WString ss = s;

    if (users_.find(ss) == users_.end())
      return ss;
  }
}

void SimpleChatServer::sendMessage(const WString& user, const WString& message)
{
  postChatEvent(ChatEvent(user, message));
}

void SimpleChatServer::postChatEvent(const ChatEvent& event)
{
  boost::recursive_mutex::scoped_lock lock(mutex_);

  WApplication *app = WApplication::instance();

  for (UserMap::const_iterator i = users_.begin(); i != users_.end(); ++i) {
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
		   boost::bind(i->second.eventCallback, event));
  }
}

SimpleChatServer::UserSet SimpleChatServer::users()
{
  boost::recursive_mutex::scoped_lock lock(mutex_);

  UserSet result;
  for (UserMap::const_iterator i = users_.begin(); i != users_.end(); ++i)
    result.insert(i->first);

  return result;
}

