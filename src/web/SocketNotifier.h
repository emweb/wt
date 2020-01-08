// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SOCKETNOTIFIER_H_
#define SOCKETNOTIFIER_H_

namespace Wt {
class WebController;
class SocketNotifierImpl;

/*
 * Class that monitors sockets using select().
 * This class invokes controller->socketSelected() when select
 * returns with activity on the socket. When this callback is
 * invoked, the socket is no longer monitored by this class
 * and it must be re-added explicitly to be monitored again.
 */
class SocketNotifier
{
public:
  SocketNotifier(WebController *controller);
  ~SocketNotifier();

  void addReadSocket(int socket);
  void addWriteSocket(int socket);
  void addExceptSocket(int socket);
  void removeReadSocket(int socket);
  void removeWriteSocket(int socket);
  void removeExceptSocket(int socket);

private:
  void startThread();
  void interruptThread();
  void threadEntry();
  void createSocketPair();

  SocketNotifierImpl *impl_;
};

}

#endif // SOCKETNOTIFIER_H_
