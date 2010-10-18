/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SimpleChatServer.h"

#include <Wt/SyncLock>

#include <iostream>
#include <boost/lexical_cast.hpp>

using namespace Wt;

const WString ChatEvent::formattedHTML(const WString& user) const
{
  switch (type_) {
  case Login:
    return "<span class='chat-info'>"
      + user_ + " joined the conversation.</span>";
  case Logout:
    return "<span class='chat-info'>"
      + ((user == user_) ? "You" : user_)
      + " logged out.</span>";
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


SimpleChatServer::SimpleChatServer()
  : chatEvent_(this)
{ }

bool SimpleChatServer::login(const WString& user)
{
  // In every application path that holds a lock to a mutex while also
  // trying to update another application (as is in this method the
  // case during chatEvent_.emit()) we need to use Wt::SyncLock to
  // avoid dead-locks.

  SyncLock<boost::recursive_mutex::scoped_lock> lock(mutex_);
  
  if (users_.find(user) == users_.end()) {
    users_.insert(user);

    chatEvent_.emit(ChatEvent(ChatEvent::Login, user));

    return true;
  } else
    return false;
}

void SimpleChatServer::logout(const WString& user)
{
  SyncLock<boost::recursive_mutex::scoped_lock> lock(mutex_);
  
  UserSet::iterator i = users_.find(user);

  if (i != users_.end()) {
    users_.erase(i);

    chatEvent_.emit(ChatEvent(ChatEvent::Logout, user));
  }
}

WString SimpleChatServer::suggestGuest()
{
  SyncLock<boost::recursive_mutex::scoped_lock> lock(mutex_);

  for (int i = 1;; ++i) {
    std::string s = "guest " + boost::lexical_cast<std::string>(i);
    WString ss = s;

    if (users_.find(ss) == users_.end())
      return ss;
  }
}

void SimpleChatServer::sendMessage(const WString& user, const WString& message)
{
  SyncLock<boost::recursive_mutex::scoped_lock> lock(mutex_);

  chatEvent_.emit(ChatEvent(user, message));
}

SimpleChatServer::UserSet SimpleChatServer::users()
{
  SyncLock<boost::recursive_mutex::scoped_lock> lock(mutex_);

  return users_;
}
