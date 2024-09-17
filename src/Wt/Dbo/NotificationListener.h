// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WT_DBONOTIFIER_H_
#define WT_DBONOTIFIER_H_

#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <map>

#include <Wt/Dbo/SqlConnection.h>

namespace Wt {

  namespace Dbo {

/*! \class NotificationListener Wt/Dbo/NotificationListener.h Wt/Dbo/NotificationListener.h
 *  \brief Class for sending and listening for notification trough a database.
 *
 * A NotificationListener send notification trough a database and
 * listen for notification from the database it is connected to. Each
 * notification is send trough a 'channel' and for each notification
 * recieved on a channel, the NotificationListener will emit a signal
 * which is unique for each channel.
 *
 * \note This class needs server-initiated updates to be enabled in
 *       order to work. This will automatically be activated when
 *       connecting to a database.
 *
 * \warning The notifications are currently only implemented for
 *          postgresql databases. In order to use notifications on
 *          other type of databases, a custom implementation of the
 *          functions \c subscribe , \c notify ,
 *          \c getNextNotify , \c setupNotify and \c stopListen must
 *          be implemented for the specific database. This can be
 *          done by inheriting from the connection to the specific
 *          database and overriding those functions.
 *
 * \ingroup dbo
 */
class WTDBO_API NotificationListener {
public:
  /*! \brief Create a NotificationListener.
   */
  NotificationListener();

  /*! \brief Create and connect a NotificationListener.
   *
   * Create a new NotificationListener and directly connect it to the
   * database usig the given connection.
   */
  explicit NotificationListener(std::unique_ptr<Dbo::SqlConnection> connection);

  /*! \brief Destroy the NotificationListener.
   */
  ~NotificationListener();

  /*! \brief Connect the NotificationListener to a database.
   *
   * Connect the NotificationListener to the database using the given
   * connection. The connection will be used exclusively by this
   * NotificationListener.
   *
   * Calling this function will destroy any connection already used
   * by the NotificationListener.
   *
   * \note This enables server-initiated updates.
   *
   * \sa disconnect()
   */
  void connect(std::unique_ptr<Dbo::SqlConnection> connection);

  /*! \brief Disconnect the NotificationListener.
   *
   * Disconnect the NotificationListener from the database it is
   * connected to.
   *
   * \note The signals for some notifications that arrived before
   *       the call of disconnect might still be emited after that
   *       call.
   */
  void disconnect();

  /*! \brief Send a notification.
   *
   * Sends a notification with the given message on the given channel.
   * If no message is given, en empty string will be sent as message.
   *
   * \sa setCallback()
   */
  void notify(const std::string& channel, const std::string& message = "");

  /*! \brief Sets a function to be called when a notification is received.
   *
   * This sets the function \p callback as the function to be called
   * when a notification is received on the given \p channel. The
   * callback function will take two strings as arguments. The first
   * string is the message received and the second string is the name
   * of the channel the notification was sent on.
   *
   * \warning Since the callback functions are called from another
   *          thread, the old function might still be called after
   *          this function returns.
   *
   * \sa removeCallback(), notify()
   */
  void setCallback(const std::string& channel,
                   const std::function<void(const std::string&, const std::string&)>& callback);

  /*! \brief Removes the callback for the given channel.
   *
   * This removes the callback function for the given channel.
   *
   * \warning Since the callback functions are called from another
   *          thread, the deleted function might still be called after
   *          this function returns.
   *
   * \sa setCallback()
   */
  void removeCallback(const std::string& channel);

private:
  std::unique_ptr<SqlConnection> connection_;
  std::atomic_bool disconnecting_;
  std::unique_ptr<std::thread> thread_;

  std::map<std::string, std::function<void(const std::string&, const std::string&)> > callbackMap_;
  std::mutex mapLock_;

  void listen();
  void handleNextNotification();
};


  }
}

#endif // WT_DBONOTIFIER_H_