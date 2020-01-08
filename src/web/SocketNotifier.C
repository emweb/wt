/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "SocketNotifier.h"
#include "WebController.h"
#include "Wt/WLogger.h"
#include "Wt/WSocketNotifier.h"
#include <set>

#if WT_WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#endif

#include <condition_variable>
#include <algorithm>


namespace Wt {

LOGGER("SocketNotifier");

class SocketNotifierImpl
{
public:
  SocketNotifierImpl():
    terminate_(false),
    socket1_(-1),
    socket2_(-1),
    controller_(nullptr),
    good_(false)
  {}
  std::thread thread_;
  std::mutex mutex_;
  bool interruptProcessed_;
  std::condition_variable interrupted_;
  bool terminate_;

  int socket1_, socket2_;
  std::set<int> readFds_, writeFds_, exceptFds_;

  WebController *controller_;

  bool good_;

  void reportError(const char *msg)
  {
#ifdef WT_WIN32
    int error = GetLastError();
#else
    int error = errno;
#endif
    LOG_ERROR(msg << ". Error code " << error);
  }
};

namespace {
  void Close(int s)
  {
#ifdef WT_WIN32
    ::closesocket(s);
#else
    ::close(s);
#endif
  }
}

SocketNotifier::SocketNotifier(WebController *controller):
  impl_(new SocketNotifierImpl)
{
  impl_->controller_ = controller;
  impl_->interruptProcessed_ = true;

  createSocketPair();
}

SocketNotifier::~SocketNotifier()
{
  impl_->terminate_ = true;
  interruptThread();
  if (impl_->thread_.joinable())
    impl_->thread_.join();
  if (impl_->socket1_ != -1)
    Close(impl_->socket1_);
  if (impl_->socket2_ != -1)
    Close(impl_->socket2_);
  delete impl_;
}

void SocketNotifier::createSocketPair()
{
  // create a socket
  int listenSocket = ::socket(PF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    impl_->reportError("create listen socket failed");
    return;
  }

  // set the nodelay option
  {
    int flag = 1;
    if (::setsockopt(listenSocket, IPPROTO_TCP, TCP_NODELAY,
          (char *)&flag, sizeof(int))) {
      // This is not fatal
      impl_->reportError("Configuring NODELAY failed");
    }
  }

  unsigned long selfAddr = 0x7f000001; // 127.0.0.1, host order
  {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(selfAddr);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    if (::bind(listenSocket, (struct sockaddr *)&addr, sizeof(addr))) {
      impl_->reportError("bind() listen socket failed");
      Close(listenSocket);
      return;
    }
  }

  unsigned listenPort; // in host byte order
  {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    if (::getsockname(listenSocket, (struct sockaddr *)&addr, &addr_size)) {
      impl_->reportError("getsockname() listen socket failed");
      Close(listenSocket);
      return;
    } else {
      selfAddr = ntohl(addr.sin_addr.s_addr);
      listenPort = ntohs(addr.sin_port);
    }
  }

  if (::listen(listenSocket, 5)) {
    impl_->reportError("listen() failed");
    Close(listenSocket);
    return;
  }

  impl_->socket1_ = ::socket(PF_INET, SOCK_STREAM, 0);
  if (impl_->socket1_ < 0) {
    impl_->reportError("create socket1 failed");
    Close(listenSocket);
    return;
  }

  // set the nodelay option
  {
    int flag = 1;
    if (::setsockopt(impl_->socket1_, IPPROTO_TCP, TCP_NODELAY,
          (char *)&flag, sizeof(int))) {
      impl_->reportError("NODELAY socket1 failed");
      // Not fatal
    }
  }

  {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listenPort);
    addr.sin_addr.s_addr = htonl(selfAddr);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    if (::connect(impl_->socket1_, (struct sockaddr *)&addr, sizeof(addr))) {
      impl_->reportError("connect socket1 failed");
      Close(listenSocket);
      Close(impl_->socket1_);
      return;
    }
  }

  unsigned connectPort;
  {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    if (::getsockname(impl_->socket1_, (struct sockaddr *)&addr, &addr_size)) {
      impl_->reportError("getsockname socket1 failed");
      Close(listenSocket);
      Close(impl_->socket1_);
      return;
    } else {
      selfAddr = ntohl(addr.sin_addr.s_addr);
      connectPort = ntohs(addr.sin_port);
    }
  }

  unsigned long peerAddr;
  unsigned peerPort;
  {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof addr;
    impl_->socket2_ =
      ::accept(listenSocket, (struct sockaddr *)&addr, &addr_size);
    if (impl_->socket2_ >= 0) {
      peerAddr = ntohl(addr.sin_addr.s_addr);
      peerPort = ntohs(addr.sin_port);
    } else {
      impl_->reportError("accept failed");
      Close(listenSocket);
      Close(impl_->socket1_);
      return;
    }
  }

  if (peerPort != connectPort && peerAddr != selfAddr) {
    // A hacker has hijacked our secret port!!
    impl_->reportError("socketpair: Accept from unexpected port");
    Close(listenSocket);
    Close(impl_->socket1_);
    Close(impl_->socket2_);
    return;
  }

  Close(listenSocket);

  // Set both sockets to non-blockin so that they won't be the cause
  // of deadlocks
#ifndef WT_WIN32
  {
    int flags = ::fcntl(impl_->socket1_, F_GETFL, 0);
    flags |= O_NONBLOCK;
    ::fcntl(impl_->socket1_, F_SETFL, flags);
  }
  {
    int flags = ::fcntl(impl_->socket2_, F_GETFL, 0);
    flags |= O_NONBLOCK;
    ::fcntl(impl_->socket2_, F_SETFL, flags);
  }
#else
  {
    u_long enabled = 1;
    ::ioctlsocket(impl_->socket1_, FIONBIO, &enabled);
    ::ioctlsocket(impl_->socket2_, FIONBIO, &enabled);
  }
#endif
  impl_->good_ = true;
}

void SocketNotifier::startThread()
{
  impl_->thread_ = std::thread(&SocketNotifier::threadEntry, this);
}

void SocketNotifier::interruptThread()
{
  if (!impl_->good_)
    return;
  if (impl_->thread_.joinable()) {
    impl_->interruptProcessed_ = false;
    char data = 0;
    sendto(impl_->socket1_, &data, 1, 0, nullptr, 0);
  } else {
    if (!impl_->terminate_) {
      // Just start the thread - there's no need for signaling
      startThread();
    }
  }
}

void SocketNotifier::threadEntry()
{
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  while (!impl_->terminate_) {
    int maxFd = 0;
    fd_set read_fds, write_fds, except_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    std::set<int> read_in = impl_->readFds_;
    std::set<int> write_in = impl_->writeFds_;
    std::set<int> except_in = impl_->exceptFds_;

    FD_SET(impl_->socket2_, &read_fds);
    maxFd = (std::max)(maxFd, impl_->socket2_);

    for (std::set<int>::const_iterator i = impl_->readFds_.begin();
         i != impl_->readFds_.end(); ++i) {
      FD_SET(*i, &read_fds);
      maxFd = (std::max)(maxFd, *i);
    }

    for (std::set<int>::const_iterator i = impl_->writeFds_.begin();
         i != impl_->writeFds_.end(); ++i) {
      FD_SET(*i, &write_fds);
      maxFd = (std::max)(maxFd, *i);
    }

    for (std::set<int>::const_iterator i = impl_->exceptFds_.begin();
         i != impl_->exceptFds_.end(); ++i) {
      FD_SET(*i, &except_fds);
      maxFd = (std::max)(maxFd, *i);
    }

    lock.unlock();
    int result = ::select(maxFd + 1, &read_fds, &write_fds, &except_fds, nullptr);
    lock.lock();
    if (result > 0) {
      if (FD_ISSET(impl_->socket2_, &read_fds)) {
        // interruption of select() was requested. Read all data from
        // the interruption connection. Normally contains a single byte,
        // unless we were interrupted a couple of times already.
        char buf[128];
        ::recvfrom(impl_->socket2_, buf, sizeof(buf), 0, nullptr, nullptr);
        // Shortcut
        if (impl_->terminate_)
          return;
      }

      // Callbacks to invoke
      std::vector<std::pair<int, WSocketNotifier::Type> > callbacks;

      // The WebController will re-enable listening after processing the event
      for (std::set<int>::iterator i = read_in.begin();
           i != read_in.end(); ++i) {
        if (FD_ISSET(*i, &read_fds)) {
          if (impl_->readFds_.find(*i) != impl_->readFds_.end()) {
            impl_->readFds_.erase(*i);
	    callbacks.push_back(std::make_pair((int)*i, 
					       WSocketNotifier::Type::Read));
          }
        }
      }

      for (std::set<int>::iterator i = write_in.begin();
           i != write_in.end(); ++i) {
        if (FD_ISSET(*i, &write_fds)) {
          if (impl_->writeFds_.find(*i) != impl_->writeFds_.end()) {
            impl_->writeFds_.erase(*i);
	    callbacks.push_back(std::make_pair((int)*i,
					       WSocketNotifier::Type::Write));
          }
        }
      }

      for (std::set<int>::iterator i = except_in.begin();
           i != except_in.end(); ++i) {
        if (FD_ISSET(*i, &except_fds)) {
          if (impl_->exceptFds_.find(*i) != impl_->exceptFds_.end()) {
            impl_->exceptFds_.erase(*i);
            callbacks.push_back(std::make_pair((int)*i,
					       WSocketNotifier::Type::Exception));
          }
        }
      }

      impl_->interruptProcessed_ = true;
      impl_->interrupted_.notify_all();
      lock.unlock();

      // Invoke callbacks
      for (unsigned int i = 0; i < callbacks.size(); ++i) {
        impl_->controller_->socketSelected(callbacks[i].first,
                                           callbacks[i].second);
      }

      lock.lock();
    } else {
      LOG_ERROR("select() returned -1");
    }
  }
}

void SocketNotifier::addReadSocket(int socket)
{
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->readFds_.insert(socket);
  interruptThread();
}

void SocketNotifier::addWriteSocket(int socket)
{
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->writeFds_.insert(socket);
  interruptThread();
}

void SocketNotifier::addExceptSocket(int socket)
{
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->exceptFds_.insert(socket);
  interruptThread();
}

void SocketNotifier::removeReadSocket(int socket)
{
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->readFds_.erase(socket);

  while (!impl_->interruptProcessed_)
    impl_->interrupted_.wait(lock);

  interruptThread();

  // In order to avoid late event invocation (especially on socket id's
  // that were recycled by the OS), we must wait until the socket was
  // really removed
  impl_->interrupted_.wait(lock);
}

void SocketNotifier::removeWriteSocket(int socket)
{
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->writeFds_.erase(socket);
  interruptThread();
  impl_->interrupted_.wait(lock);
}

void SocketNotifier::removeExceptSocket(int socket)
{
  std::unique_lock<std::mutex> lock(impl_->mutex_);
  impl_->exceptFds_.erase(socket);
  interruptThread();
  impl_->interrupted_.wait(lock);
}

}
