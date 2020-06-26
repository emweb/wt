/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "SessionProcess.h"

#ifdef __linux__
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifndef WT_WIN32
#include <signal.h>
#endif // WT_WIN32

#include "Wt/WConfig.h"
#include "Wt/WLogger.h"

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
    Wt::AsioWrapper::error_code ignored_ec;
    socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
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
			       const std::function<void (bool)>& onReady)
{
  asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::loopback(), 0);
  Wt::AsioWrapper::error_code ec;
  acceptor_->open(endpoint.protocol(), ec);
  if (!ec)
    acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (!ec)
    acceptor_->bind(endpoint, ec);
  if (!ec)
    acceptor_->listen(0, ec);
#ifndef WT_WIN32
#if (defined(WT_ASIO_IS_BOOST_ASIO) && BOOST_VERSION >= 106600) || (defined(WT_ASIO_IS_STANDALONE_ASIO) && ASIO_VERSION >= 101100)
  fcntl(acceptor_->native_handle(), F_SETFD, FD_CLOEXEC);
#else
  fcntl(acceptor_->native(), F_SETFD, FD_CLOEXEC);
#endif
#endif // !WT_WIN32
  if (ec) {
    LOG_ERROR("Couldn't create listening socket: " << ec.message());
    if (!onReady) {
      onReady(false);
      return;
    }
  }
  acceptor_->async_accept
    (*socket_, std::bind(&SessionProcess::acceptHandler, shared_from_this(),
			 std::placeholders::_1, onReady));
  LOG_DEBUG("Listening to child process on port " 
	    << acceptor_->local_endpoint(ec).port());

  exec(config, onReady);
}

void SessionProcess::acceptHandler(const Wt::AsioWrapper::error_code& err,
				   const std::function<void (bool)>& onReady)
{
  if (!err) {
    acceptor_.reset();
    asio::async_read
      (*socket_, asio::buffer(buf_, 5),
       std::bind(&SessionProcess::readPortHandler, shared_from_this(),
		 std::placeholders::_1,
		 std::placeholders::_2,
		 onReady));
  }
}

void SessionProcess::readPortHandler(const Wt::AsioWrapper::error_code& err,
				     std::size_t transferred,
				     const std::function<void (bool)>& onReady)
{
  if (!err || err == asio::error::eof || err == asio::error::shut_down) {
    closeClientSocket();
    buf_[transferred] = '\0';
    port_ = atoi(buf_);
    LOG_DEBUG("Child is listening on port " << port_);
    if (onReady) {
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

#ifdef WT_WIN32
namespace {

// Quoting argument function, modified from
// https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
void appendArgToCmdLine(const std::string &arg, std::wstring &commandLine)
{
  const int argwSize = MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, arg.c_str(), arg.size(), (LPWSTR)0, 0);
  std::wstring argw(argwSize, L'\0');
  MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, arg.c_str(), arg.size(), &argw[0], argw.size());

  if (!argw.empty() &&
      argw.find_first_of(L" \t\n\v\"") == std::wstring::npos)
    commandLine.append(argw);
  else {
    commandLine.push_back(L'"');

    for (std::wstring::const_iterator it = argw.begin(); ; ++it) {
      int number_backslashes = 0;

      while (it != argw.end() && *it == L'\\') {
        ++it;
        ++number_backslashes;
      }

      if (it == argw.end()) {
        // Escape all backslashes, but let the terminating
        // double quotation mark we add below be interpreted
        // as a metacharacter.
        commandLine.append(number_backslashes * 2, L'\\');
        break;
      } else if (*it == L'"') {
        // Escape all backslashes and the following
        // double quotation mark.
        commandLine.append(number_backslashes * 2 + 1, L'\\');
        commandLine.push_back(*it);
      } else {
        // Backslashes aren't special here.
        commandLine.append(number_backslashes, L'\\');
        commandLine.push_back(*it);
      }
    }

    commandLine.push_back(L'"');
  }

  commandLine.push_back(L' ');
}

}
#endif // WT_WIN32

void SessionProcess::exec(const Configuration& config,
			  const std::function<void (bool)>& onReady)
{
#ifndef WT_WIN32
  std::string parentPortOption = std::string("--parent-port=")
    + std::to_string(acceptor_->local_endpoint().port());
  const std::vector<std::string> &options = config.options();
  const char **c_options = new const char*[options.size() + 2];
  std::size_t i = 0;
  for (; i < options.size(); ++i) {
    c_options[i] = options[i].c_str();
  }
  c_options[i] = parentPortOption.c_str();
  ++i;
  c_options[i] = 0;

  pid_ = fork();
  if (pid_ < 0) {
    LOG_ERROR("failed to fork dedicated session process, error code: " << errno);
    stop();
    if (onReady)
      onReady(false);
    delete[] c_options;
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
  }

  delete[] c_options;
#else // WT_WIN32
  std::wstring commandLine;

  const std::vector<std::string> &options = config.options();
  for (std::size_t i = 0; i < options.size(); ++i) {
    appendArgToCmdLine(options[i], commandLine);
  }

  std::wstring parentPortOption = std::wstring(L"--parent-port=")
    + boost::lexical_cast<std::wstring>(acceptor_->local_endpoint().port());
  commandLine += parentPortOption;

  LPWSTR c_commandLine = new wchar_t[commandLine.size() + 1];
  wcscpy(c_commandLine, commandLine.c_str());

  STARTUPINFOW startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);

  if(!CreateProcessW(0, c_commandLine, 0, 0, true,
      0, 0, 0, &startupInfo, &processInfo_)) {
    LOG_ERROR("failed to start dedicated session process, error code: " << GetLastError());
    stop();
    if (onReady) {
      onReady(false);
    }
  }
  delete[] c_commandLine;
#endif // WT_WIN32
}

} // namespace server
} // namespace http
