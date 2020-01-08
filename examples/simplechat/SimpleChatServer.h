// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SIMPLECHATSERVER_H_
#define SIMPLECHATSERVER_H_

#include <Wt/WSignal.h>
#include <Wt/WString.h>

namespace Wt {
  class WServer;
}

#include <set>
#include <map>
#include <thread>
#include <mutex>

/**
 * @addtogroup chatexample
 */
/*@{*/

/*! \brief Encapsulate a chat event.
 */
class ChatEvent
{
public:
  /*! \brief Enumeration for the event type.
   */
  enum Type { Login, Logout, Rename, Message };

  /*! \brief Get the event type.
   */
  Type type() const { return type_; }

  /*! \brief Get the user who caused the event.
   */
  const Wt::WString& user() const { return user_; }

  /*! \brief Get the message of the event.
   */
  const Wt::WString& message() const { return message_; }

  /*! \brief Get the extra data for this event.
   */
  const Wt::WString& data() const { return data_; }

  /*! \brief Get the message formatted as HTML, rendered for the given user.
   *
   * The \p format indicates how the message should be formatted.
   */
  const Wt::WString formattedHTML(const Wt::WString& user,
				  Wt::TextFormat format) const;

private:
  Type    type_;
  Wt::WString user_;
  Wt::WString data_;
  Wt::WString message_;

  /*
   * Both user and html will be formatted as html
   */
  ChatEvent(const Wt::WString& user, const Wt::WString& message)
    : type_(Message), user_(user), message_(message)
  { }

  ChatEvent(Type type, const Wt::WString& user,
	    const Wt::WString& data = Wt::WString::Empty)
    : type_(type), user_(user), data_(data)
  { }

  friend class SimpleChatServer;
};

typedef std::function<void (const ChatEvent&)> ChatEventCallback;

/*! \brief A simple chat server
 */
class SimpleChatServer
{
public:
  /*
   * A reference to a client.
   */
  class Client
  {
  };

  /*! \brief Create a new chat server.
   */
  SimpleChatServer(Wt::WServer& server);

  SimpleChatServer(const SimpleChatServer &) = delete;
  SimpleChatServer &operator=(const SimpleChatServer &) = delete;

  /*! \brief Connects to the chat server.
   *
   * The passed callback method is posted to when a new chat event is
   * received.
   *
   * Returns whether the client has been connected (or false if the client
   * was already connected).
   */
  bool connect(Client *client, const ChatEventCallback& handleEvent);

  /*! \brief Disconnect from the chat server.
   *
   * Returns whether the client has been disconnected (or false if the client
   * was not connected).
   */  
  bool disconnect(Client *client);

  /*! \brief Try to login with given user name.
   *
   * Returns false if the login was not successful.
   */
  bool login(const Wt::WString& user);

  /*! \brief Logout from the server.
   */
  void logout(const Wt::WString& user);

  /*! \brief Changes the name.
   */
  bool changeName(const Wt::WString& user, const Wt::WString& newUser);

  /*! \brief Get a suggestion for a guest user name.
   */
  Wt::WString suggestGuest();

  /*! \brief Send a message on behalve of a user.
   */
  void sendMessage(const Wt::WString& user, const Wt::WString& message);

  /*! \brief Typedef for a collection of user names.
   */
  typedef std::set<Wt::WString> UserSet;

  /*! \brief Get the users currently logged in.
   */
  UserSet users();

private:
  struct ClientInfo {
    std::string       sessionId;
    ChatEventCallback eventCallback;
  };

  typedef std::map<Client *, ClientInfo> ClientMap;

  Wt::WServer&                server_;
  std::recursive_mutex    mutex_;
  ClientMap               clients_;
  UserSet                 users_;

  void postChatEvent(const ChatEvent& event);
};

/*@}*/

#endif // SIMPLECHATSERVER_H_
