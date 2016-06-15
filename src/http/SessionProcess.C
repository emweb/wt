/*
 * Copyright (C) 2014 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "SessionProcess.h"

#include <boost/bind.hpp>

#ifdef __linux__
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifndef WT_WIN32
#include <signal.h>
#endif // WT_WIN32

#include "Wt/WLogger"

namespace Wt {
  LOGGER("wthttp/proxy");
}

namespace http {
namespace server {

SessionProcess::SessionProcess(asio::io_service &io_service)
  : io_service_(io_service),
    socket_(new asio::ip::tcp::socket(io_service)),
    acceptor_(new asio::ip::tcp::acceptor(io_service)),
    port_(-1)
#ifndef WT_WIN32
    ,pid_(0)
#endif // !WT_WIN32
{
#ifdef WT_WIN32
  ZeroMemory(&processInfo_, sizeof(processInfo_));
#endif // WT_WIN32
}

void SessionProcess::closeClientSocket()
{
  if (socket_.get()) {
    boost::system::error_code ignored_ec;
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    socket_->close();
    socket_.reset();
  }
  if (acceptor_.get()) {
    acceptor_->cancel();
    acceptor_->close();
    acceptor_.reset();
  }
}

void SessionProcess::stop()
{
  closeClientSocket();
#ifdef WT_WIN32
  if (processInfo_.hProcess != 0) {
    LOG_DEBUG("Closing handles to process " << processInfo_.dwProcessId);
    CloseHandle(processInfo_.hProcess);
    CloseHandle(processInfo_.hThread);
    ZeroMemory(&processInfo_, sizeof(processInfo_));
  }
#endif // WT_WIN32
}

void SessionProcess::asyncExec(const Configuration &config,
			       boost::function<void (bool)> onReady)
{
  asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::loopback(), 0);
  boost::system::error_code ec;
  acceptor_->open(endpoint.protocol(), ec);
  if (!ec)
    acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (!ec)
    acceptor_->bind(endpoint, ec);
  if (!ec)
    acceptor_->listen(0, ec);
#ifndef WT_WIN32
  fcntl(acceptor_->native(), F_SETFD, FD_CLOEXEC);
#endif // !WT_WIN32
  if (ec) {
    LOG_ERROR("Couldn't create listening socket: " << ec.message());
    if (!onReady.empty()) {
      onReady(false);
      return;
    }
  }
  acceptor_->async_accept(*socket_,
      boost::bind(&SessionProcess::acceptHandler, shared_from_this(),
	asio::placeholders::error, onReady));
  LOG_DEBUG("Listening to child process on port " << acceptor_->local_endpoint(ec).port());

  exec(config, onReady);
}

void SessionProcess::acceptHandler(const boost::system::error_code& err,
				   boost::function<void (bool)> onReady)
{
  if (!err) {
    acceptor_.reset();
    asio::async_read(*socket_, asio::buffer(buf_, 5),
	boost::bind(&SessionProcess::readPortHandler, shared_from_this(),
	  asio::placeholders::error,
	  asio::placeholders::bytes_transferred,
	  onReady));
  }
}

void SessionProcess::readPortHandler(const boost::system::error_code& err,
				     std::size_t transferred,
				     boost::function<void (bool)> onReady)
{
  if (!err || err == asio::error::eof || err == asio::error::shut_down) {
    closeClientSocket();
    buf_[transferred] = '\0';
    port_ = atoi(buf_);
    LOG_DEBUG("Child is listening on port " << port_);
    if (!onReady.empty()) {
      onReady(true);
    }
  }
}

void SessionProcess::setSessionId(const std::string& sessionId)
{
  sessionId_ = sessionId;
}

asio::ip::tcp::endpoint SessionProcess::endpoint() const
{
  return asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), port_);
}

void SessionProcess::exec(const Configuration& config,
			  boost::function<void (bool)> onReady)
{
#ifndef WT_WIN32
  std::string parentPortOption = std::string("--parent-port=")
    + boost::lexical_cast<std::string>(acceptor_->local_endpoint().port());
  const std::vector<std::string> &options = config.options();
  const char **c_options = new const char*[options.size() + 2];
  std::size_t i = 0;
  for (; i < options.size(); ++i) {
    c_options[i] = options[i].c_str();
  }
  c_options[i] = parentPortOption.c_str();
  ++i;
  c_options[i] = 0;

#if BOOST_VERSION >= 104700
  io_service_.notify_fork(boost::asio::io_service::fork_prepare);
#endif

  pid_ = fork();
  if (pid_ < 0) {
    LOG_ERROR("failed to fork dedicated session process, error code: " << errno);
    stop();
    if (!onReady.empty()) {
      onReady(false);
    }
    return;
  } else if (pid_ == 0) {
    /* child process */

    /*
     * CAREFUL! cannot use any library call here, since malloc(),
     * etc... may dead-lock.
     */

#ifdef __linux__
    /*
     * Close all (possibly) open file descriptors.
     * This could also work on other UNIX flavours...
     */
    struct rlimit l;
    getrlimit(RLIMIT_NOFILE, &l);
    for (unsigned i = 3; i < l.rlim_cur; ++i) {
      close(i);
    }
#endif

#ifdef WT_THREADED
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_UNBLOCK, &mask, 0);
#endif // WT_THREADED
    

    execv(c_options[0], const_cast<char *const *>(c_options));
    // An error occurred, this should not be reached
    exit(1);
  } else {
    /* parent process */
#if BOOST_VERSION >= 104700
    io_service_.notify_fork(boost::asio::io_service::fork_parent);
#endif
  }

  delete[] c_options;
#else // WT_WIN32
  STARTUPINFOW startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);


  std::wstring commandLine = GetCommandLineW();
  commandLine += L" --parent-port=" + boost::lexical_cast<std::wstring>(acceptor_->local_endpoint().port());
  LPWSTR c_commandLine = new wchar_t[commandLine.size() + 1];
  wcscpy(c_commandLine, commandLine.c_str());

  if(!CreateProcessW(0, c_commandLine, 0, 0, true,
      0, 0, 0, &startupInfo, &processInfo_)) {
    LOG_ERROR("failed to start dedicated session process, error code: " << GetLastError());
    stop();
    if (!onReady.empty()) {
      onReady(false);
    }
  }
  delete c_commandLine;
#endif // WT_WIN32
}

} // namespace server
} // namespace http
