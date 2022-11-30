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
#include <csignal>
#endif // WT_WIN32

#include "Wt/WConfig.h"
#include "Wt/WLogger.h"

#include "web/WebUtils.h"

#include "SessionProcessManager.h"

namespace Wt {
  LOGGER("wthttp/proxy");
}

namespace http {
namespace server {

SessionProcess::SessionProcess(SessionProcessManager *manager) noexcept
  : io_service_(manager->ioService()),
    socket_(new asio::ip::tcp::socket(io_service_)),
    acceptor_(new asio::ip::tcp::acceptor(io_service_)),
    strand_(io_service_),
    port_(-1),
#ifndef WT_WIN32
    pid_(0),
#endif // !WT_WIN32
    manager_(manager)
{
#ifdef WT_WIN32
  ZeroMemory(&processInfo_, sizeof(processInfo_));
#endif // WT_WIN32
}

void SessionProcess::requestStop() noexcept
{
  io_service_.post(strand_.wrap(
          std::bind(&SessionProcess::stop, shared_from_this())));
}

void SessionProcess::closeClientSocket() noexcept
{
  Wt::AsioWrapper::error_code ignored_ec;
  if (socket_) {
    socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
    socket_->close(ignored_ec);
    socket_ = nullptr;
  }
  if (acceptor_) {
    acceptor_->cancel(ignored_ec);
    acceptor_->close(ignored_ec);
    acceptor_ = nullptr;
  }
}

void SessionProcess::stop() noexcept
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
                               const std::function<void (bool)>& onReady) noexcept
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
    onReady(false);
    return;
  }
  acceptor_->async_accept
    (*socket_, strand_.wrap(
            std::bind(&SessionProcess::acceptHandler, shared_from_this(), std::placeholders::_1, onReady)));
  LOG_DEBUG("Listening to child process on port "
            << acceptor_->local_endpoint(ec).port());

  exec(config, onReady);
}

void SessionProcess::acceptHandler(const Wt::AsioWrapper::error_code& err,
                                   const std::function<void (bool)>& onReady) noexcept
{
  if (!err) {
    acceptor_ = nullptr;

    listeningCallback_ = onReady;
    read();
  }
}

void SessionProcess::read() noexcept
{
  asio::async_read_until
    (*socket_, buf_, '\n',
     strand_.wrap(
       std::bind(&SessionProcess::readHandler, shared_from_this(), std::placeholders::_1)));
}

void SessionProcess::readHandler(const Wt::AsioWrapper::error_code& err) noexcept
{
  if (!err) {
    std::istream is(&buf_);
    std::string msg;
    std::getline(is, msg);

    if (!handleChildMessage(msg)) {
      closeClientSocket();
      return;
    } else if (port_ == -1) {
      LOG_ERROR("could not read child process listening port");
      closeClientSocket();
      return;
    } else if (listeningCallback_) {
      LOG_DEBUG("Child is listening on port " << port_);
      listeningCallback_(true);
      listeningCallback_ = nullptr;
    }

    read();
  } else {
    closeClientSocket();
  }
}

bool SessionProcess::handleChildMessage(const std::string& message) noexcept
{
  auto idx = message.find_first_of(':');
  if (idx == std::string::npos) {
    LOG_ERROR("received invalid message from child process: " << message);
    return false;
  }

  auto key = message.substr(0, idx);
  auto value = message.substr(idx+1);

  if (key == "port") {
    try {
      port_ = Wt::Utils::stoi(value);
    } catch (std::invalid_argument& e) {
      LOG_ERROR("invalid listening port: " << e.what());
      return false;
    }
  } else if (key == "session-id") {
    if (manager_)
      manager_->updateSessionId(value, shared_from_this());
  } else {
    LOG_ERROR("received invalid message from child process: " << message);
    return false;
  }

  return true;
}

void SessionProcess::setSessionId(const std::string& sessionId) noexcept
{
  sessionId_ = sessionId;
}

asio::ip::tcp::endpoint SessionProcess::endpoint() const noexcept
{
  return asio::ip::tcp::endpoint(asio::ip::address_v4::loopback(), port_);
}

#ifdef WT_WIN32
namespace {

// Quoting argument function, modified from
// https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
void appendArgToCmdLine(const std::string &arg, std::wstring &commandLine) noexcept
{
  const int argwSize = MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, arg.c_str(), arg.size(), nullptr, 0);
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
                          const std::function<void (bool)>& onReady) noexcept
{
#ifndef WT_WIN32
  std::string parentPortOption = std::string("--parent-port=")
    + std::to_string(acceptor_->local_endpoint().port());
  const std::vector<std::string> &options = config.options();
  std::unique_ptr<const char*[]> c_options(new const char*[options.size() + 2]);
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
    onReady(false);
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
    struct rlimit l{};
    getrlimit(RLIMIT_NOFILE, &l);
    for (unsigned fd = 3; fd < l.rlim_cur; ++fd) {
      close(static_cast<int>(fd));
    }
#endif

#ifdef WT_THREADED
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_UNBLOCK, &mask, nullptr);
#endif // WT_THREADED


    execv(c_options[0], const_cast<char *const *>(c_options.get()));
    // An error occurred, this should not be reached
    exit(1);
  }
#else // WT_WIN32
  std::wstring commandLine;

  for (const auto& option : config.options()) {
    appendArgToCmdLine(option, commandLine);
  }

  std::wstring parentPortOption = std::wstring(L"--parent-port=") + std::to_wstring(acceptor_->local_endpoint().port());
  commandLine += parentPortOption;

  std::unique_ptr<WCHAR[]> c_commandLine(new WCHAR[commandLine.size() + 1]);
  wcscpy(c_commandLine.get(), commandLine.c_str());

  STARTUPINFOW startupInfo;
  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);

  if(!CreateProcessW(
        nullptr,
        c_commandLine.get(),
        nullptr,
        nullptr,
        true,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo_)) {
    LOG_ERROR("failed to start dedicated session process, error code: " << GetLastError());
    stop();
    onReady(false);
  }
#endif // WT_WIN32
}

} // namespace server
} // namespace http
