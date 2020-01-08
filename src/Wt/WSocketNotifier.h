// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSOCKETNOTIFIER_H_
#define WSOCKETNOTIFIER_H_

#include <Wt/WObject.h>
#include <Wt/WSignal.h>

namespace Wt {

/*! \class WSocketNotifier Wt/WSocketNotifier.h Wt/WSocketNotifier.h
 *  \brief A utility class for asynchronous notification of socket activity.
 *
 * Use a socket notifier to integrate listening for socket events into
 * the %Wt event loop. In this way, you do not need a separate thread
 * to listen for socket activity. Socket activity is either the
 * availability of data to be read (\link WSocketNotifier::Read Read
 * event\endlink), possibility to write data (\link
 * WSocketNotifier::Write Write event\endlink), or an exception that
 * occurred (\link WSocketNotifier::Exception Exception
 * event\endlink).
 *
 * When an event on a socket is available, the notifier emits the
 * activated() signal. As in the case of a user interface event (like
 * for example WInteractWidget::clicked()), you will typically modify
 * the widget tree in response to the event. But, unless you use a
 * timer (WTimer) or use server-initiated updates (see
 * WApplication::triggerUpdates()), these changes are not propagated
 * to the user interface, until the next user interface event.
 *
 * Like other events, socket notification events are serial (not
 * simultaneous), and there are no thread safety issues (you don't
 * need to take the WApplication::UpdateLock).
 *
 * \code
 * std::unique_ptr<Wt::WSocketNotifier> notifier_;
 *
 * void init() {
 *   ...
 *   int sock = ...
 *   notifier_ = std::make_unique<Wt::WSocketNotifier>(sock, Wt::WSocketNotifier::Read);
 *   notifier_->activated().connect(this, &HelloApplication::readData);
 * }
 *
 * void readData() {
 *   // data is available on socket, or socket was closed by peer
 *   char buf[100];
 *   int s = read(notifier_->socket(), buf, 99);
 *
 *   if (s > 0) {
 *     ...
 *   } else {
 *     // closed by peer
 *     notifier_->setEnabled(false);
 *     close(notifier_->socket());
 *   }
 * }
 *
 * \endcode
 */
class WT_API WSocketNotifier : public WObject
{
public:
  /*! \brief Enumeration that event type.
   */
  enum class Type {
    Read,      //!< Ready to read
    Write,     //!< Ready to write
    Exception  //!< Exception
  };

  /*! \brief Creates a new socket notifier.
   *
   * Create a new socket listener to listen for events of given \p type
   * on a socket with file descriptor \p socket. The WSocketNotifier is
   * enabled after construction.
   */
  WSocketNotifier(int socket, Type type);

  /*! \brief Destructor.
   */
  ~WSocketNotifier();

  /*! \brief Returns the socket.
   */
  int socket() const { return socket_; }

  /*! \brief Returns the event type.
   */
  Type type() const { return type_; }

  /*! \brief Enables or disable the notifier.
   *
   * By default, the socket notifier is enabled to receive
   * events. When disabled, no events will be notified (as if the
   * socket notifier didn't exist).
   */
  void setEnabled(bool enabled);

  /*! \brief Returns if the notifier is enabled.
   */
  bool isEnabled() const { return enabled_; }

  /*! \brief %Signal indicating an event.
   *
   * The signal is emitted when an event that was waited for is
   * available. The signal argument is socket().
   */
  Signal<int>& activated() { return activated_; }

private:
  int socket_;
  Type type_;
  bool enabled_;
  bool beingNotified_;
  std::string sessionId_;

  Signal<int> activated_;

  const std::string& sessionId() const { return sessionId_; }
  void notify();
  void dummy();

  friend class WebController;
};

}

#endif // WSOCKETNOTIFIER_H_
