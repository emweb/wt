// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SIMPLECHATSERVER_H_
#define SIMPLECHATSERVER_H_

#include <Wt/WObject>
#include <Wt/WSignal>
#include <Wt/WString>

#include <set>
#include <boost/thread.hpp>

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
  enum Type { Login, Logout, Message };

  /*! \brief Get the event type.
   */
  Type type() const { return type_; }

  /*! \brief Get the user who caused the event.
   */
  const Wt::WString& user() const { return user_; }

  /*! \brief Get the message of the event.
   */
  const Wt::WString& message() const { return message_; }

  /*! \brief Get the message formatted as HTML, rendered for the given user.
   */
  const Wt::WString formattedHTML(const Wt::WString& user) const;

private:
  Type type_;
  Wt::WString user_;
  Wt::WString message_;

  /*
   * Both user and html will be formatted as html
   */
  ChatEvent(const Wt::WString& user, const Wt::WString& message)
    : type_(Message), user_(user), message_(message)
  { }

  ChatEvent(Type type, const Wt::WString& user)
    : type_(type), user_(user)
  { }

  friend class SimpleChatServer;
};

/*! \brief A simple chat server
 */
class SimpleChatServer : public Wt::WObject
{
public:
  /*! \brief Create a new chat server.
   */
  SimpleChatServer();

  /*! \brief Try to login with given user name.
   *
   * Returns false if the login was not successfull.
   */
  bool login(const Wt::WString& user);

  /*! \brief Logout from the server.
   */
  void logout(const Wt::WString& user);

  /*! \brief Get a suggestion for a guest user name.
   */
  Wt::WString suggestGuest();

  /*! \brief Send a message on behalve of a user.
   */
  void sendMessage(const Wt::WString& user, const Wt::WString& message);

  /*! \brief %Signal that will convey chat events.
   *
   * Every client should connect to this signal, and process events.
   */
  Wt::Signal<ChatEvent>& chatEvent() { return chatEvent_; }

  /*! \brief Typedef for a collection of user names.
   */
  typedef std::set<Wt::WString> UserSet;

  /*! \brief Get the users currently logged in.
   */
  UserSet users();

private:
  Wt::Signal<ChatEvent>         chatEvent_;
  boost::recursive_mutex        mutex_;

  UserSet                       users_;
};

/*@}*/

#endif // SIMPLECHATSERVER_H_
