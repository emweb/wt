/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WSocketNotifier.h>
#include <Wt/WText.h>

#include <algorithm>
#include <iostream>

#if WT_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define HOSTNAME "www.webtoolkit.eu"
#define URL_PATH "/wt/blog/feed/"

using namespace Wt;

/*
 * This is a minimal socket notifier example, which is used to asynchronously
 * read an RSS file and display it in raw format on the browser using server
 * push.
 * Note that when the SocketNotifier signal is emitted, that Wt already
 * conveniently grabbed the update lock for you. You can simply modify
 * the widget tree and use the server push mechanism to push the changes
 * to the browser.
 * The example looks unnecessary complex due to the use of the raw POSIX
 * socket functions. Usually these are wrapped in a more programmer-friendly
 * API.
 */

class RssReader : public WContainerWidget
{
public:
  RssReader()
    : WContainerWidget(),
      state_(CONNECT),
      bytesSent_(0)
  {
    this->addWidget(cpp14::make_unique<WText>("Click 'Start' to download the Wt homepage RSS feed<br/>"
      "The download will be done asynchronously and this page will update its "
      "contents to inform you about the progress using server push.<br/>"));
    startButton_ = this->addWidget(cpp14::make_unique<WPushButton>("Start"));
    startButton_->clicked().connect(startButton_, &WPushButton::disable);
    startButton_->clicked().connect(this, &RssReader::startDownload);

    resultText_ = this->addWidget(cpp14::make_unique<WText>());
    resultText_->setInline(false);
    rssText_ = this->addWidget(cpp14::make_unique<WText>());
    rssText_->setInline(false);
  }

private:
  int socket_;
  enum {CONNECT, WRITE, READ} state_;
  int bytesSent_;
  std::unique_ptr<WSocketNotifier> readNotifier_, writeNotifier_;
  WPushButton *startButton_;
  WText *resultText_;
  WText *rssText_;
  std::stringstream inStream_;

  // Convenience function that updates the status message.
  void addText(const WString &text)
  {
    resultText_->setText(resultText_->text() + text);
    if (wApp->updatesEnabled())
      wApp->triggerUpdate();

  }

  void startDownload()
  {
    // Enable server push
    wApp->enableUpdates(true);

    rssText_->setText("");
    resultText_->setText("");
    addText("Resolving hostname...");
    startButton_->setText("Busy...");
    /*
     * Name resolving may take a while, so entertain the user
     * already with the updates so far. As this slot is invoked
     * by the browser (caused by the user clicking the 'start'
     * button), we can use processEvents() to send the changes
     * we made to the widget tree up till here back to the browser
     */
    wApp->processEvents();

    struct addrinfo *info;
    if (getaddrinfo(HOSTNAME, "http", 0, &info) == 0) {
      socket_ = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
      /*
       * Install notifiers for read and write events. These will end up
       * in a call to select, and activated() will be called whenever
       * select decides that the socket is ready for read or write.
       */
      readNotifier_ = cpp14::make_unique<WSocketNotifier>(socket_,
          WSocketNotifier::Type::Read);
      readNotifier_->setEnabled(false); // Linux fires this on connect, weird
      readNotifier_->activated().connect(this, &RssReader::read);
      writeNotifier_ = cpp14::make_unique<WSocketNotifier>(socket_,
          WSocketNotifier::Type::Write);
      writeNotifier_->activated().connect(this, &RssReader::write);

      // Set sockets to non-blocking
#ifndef WT_WIN32
      int flags = ::fcntl(socket_, F_GETFL, 0);
      flags |= O_NONBLOCK;
      ::fcntl(socket_, F_SETFL, flags);
#else
      u_long enabled = 1;
      ::ioctlsocket(socket_, FIONBIO, &enabled);
#endif
      // Perform a non-blocking connect. POSIX specifies that the socket
      // will be marked as ready for write when the connect call completes.
      int err = ::connect(socket_, info->ai_addr, info->ai_addrlen);
#ifndef WT_WIN32
      int err2 = errno;
#else
      int err2 = GetLastError();
#endif
      freeaddrinfo(info);

      addText(" Done!<br/>Connecting...");
      wApp->processEvents();

      if (err == 0) {
        // connected, proceed immediately to 'writing'
        state_ = WRITE;
        // write() will be invoked automatically by the notifier.
      } else if (err == -1) {
#ifndef WT_WIN32
        if (err2 == EINPROGRESS) {
#else
        if (err2 == WSAEWOULDBLOCK) {
#endif
          state_ = CONNECT;
          // The writeNotifier will be fired when connected
        } else {
          addText(" Problem with connect(). Giving up.<br/>");
          cleanup();
        }
      }
    } else {
      addText("Terminating: could not resolve web service host: " HOSTNAME);
      cleanup();
    }
  }

  void write()
  {
    const char request[] = "GET " URL_PATH " HTTP/1.1\r\n"
      "Host: " HOSTNAME "\r\n"
      "Connection: Close\r\n\r\n";

    switch(state_) {
    case CONNECT:
      {
        int err;
        socklen_t len = sizeof err;
        getsockopt(socket_, SOL_SOCKET, SO_ERROR, (char *)&err, &len);
        if (err != 0) {
          addText(" connect() failed. Giving up.<br/>");
          cleanup();
        } else {
          addText(" Connected!<br/>Writing data");
          state_ = WRITE;
        }
      }
      break;
    case WRITE:
      {
        addText(".");
        // write in bits for demonstration purposes only
        int len = (std::min<int>)(10, (sizeof request) - bytesSent_);
        int retval = ::send(socket_, request + bytesSent_, len, 0);
        if (retval >= 0) {
          bytesSent_ += retval;
          if (bytesSent_ >= (int)(sizeof request)) {
            addText(" Done!<br/>Reading data");
            state_ = READ;
            // We don't need any further notifications that we can
            // keep writing
            writeNotifier_->setEnabled(false);
            readNotifier_->setEnabled(true);
          }
        } else {
#ifndef WT_WIN32
          if (errno != EAGAIN) {
#else
          if (GetLastError() == WSAEWOULDBLOCK) {
#endif
            addText("send() failed. Giving up.<br/>");
            cleanup();
          }
        }
      }
      break;
      case READ:
	break;
    }
  }

  void read()
  {
    addText(".");
    char buf[128];
    int retval = ::recv(socket_, buf, sizeof buf, 0);
    if (retval == 0) {
      // 'orderly shutdown'
      addText(" Done! (Remote end closed connection)<br/>");
      cleanup();
    } else if (retval < 0) {
#ifndef WT_WIN32
      if (errno != EAGAIN) {
#else
      if (GetLastError() == WSAEWOULDBLOCK) {
#endif
        // Euh.. all done?
        addText("recv failed. Giving up.<br/>");
        cleanup();
      }
    } else {
      inStream_.write(buf, retval);
    }
  }

  void cleanup()
  {
    /*
     * It is mandatory not to have notifiers on closed sockets,
     * as select() fails miserably in this case. Disable (or even
     * better, delete) the notifiers before you close the sockets.
     */
    readNotifier_.reset();
    writeNotifier_.reset();
#ifdef WT_WIN32
    closesocket(socket_);
#else
    close(socket_);
#endif
    socket_ = 0;
    state_ = CONNECT;
    bytesSent_ = 0;
    startButton_->setText("Again");
    startButton_->enable();
    rssText_->setText("<pre>" +
        escapeText(inStream_.str()) + "</pre>");
    addText("Finished!<br/>Run again?<br/>");
    inStream_.str("");
    wApp->enableUpdates(false);
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  std::unique_ptr<WApplication> app
      = cpp14::make_unique<WApplication>(env);
  app->root()->addWidget(cpp14::make_unique<RssReader>());

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
