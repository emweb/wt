// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_SESSION_PROCESS_HPP
#define WT_SESSION_PROCESS_HPP

#include "Wt/WConfig.h"

#include "Wt/AsioWrapper/asio.hpp"
#include "Wt/AsioWrapper/system_error.hpp"

#include "Configuration.h"

#ifdef WT_THREADED
#include <mutex>
#endif // WT_THREADED

#ifndef WT_WIN32
#include <sys/types.h>
#endif // WT_WIN32

namespace http {
namespace server {

class SessionProcessManager;

namespace asio = Wt::AsioWrapper::asio;

class SessionProcess
  : public std::enable_shared_from_this<SessionProcess>
{
public:
  explicit SessionProcess(SessionProcessManager *manager) noexcept;

  SessionProcess(const SessionProcess&) = delete;
  SessionProcess& operator=(const SessionProcess&) = delete;
  SessionProcess(SessionProcess&&) = delete;
  SessionProcess& operator=(SessionProcess&&) = delete;

  /*! \internal
   *  \brief Posts stop() to the io context, guarded with a strand
   */
  void requestStop() noexcept;

  // Execute the session process, and call the onReady callback
  // function when done. The bool passed on to the onReady function
  // indicates success.
  void asyncExec(const Configuration &config,
                 const std::function<void (bool)>& onReady) noexcept;

  // Check whether the process is ready to accept connections
  bool ready() const noexcept { return port_ != -1; }
  int port() const noexcept { return port_; }
#ifndef WT_WIN32
  pid_t pid() const noexcept { return pid_; }
#else // WT_WIN32
  DWORD pid() const noexcept { return processInfo_.dwProcessId; }
  PROCESS_INFORMATION& processInfo() noexcept { return processInfo_; }
#endif // WT_WIN32

  const std::string& sessionId() const noexcept { return sessionId_; }
  void setSessionId(const std::string& sessionId) noexcept;

  // Get the endpoint to connect to
  asio::ip::tcp::endpoint endpoint() const noexcept;

private:
  void stop() noexcept;
  void closeClientSocket() noexcept;
  void exec(const Configuration& config,
            const std::function<void (bool)>& onReady) noexcept;
  void acceptHandler(const Wt::AsioWrapper::error_code& err,
                     const std::function<void (bool)>& onReady) noexcept;
  void read() noexcept;
  void readHandler(const Wt::AsioWrapper::error_code& err) noexcept;
  bool handleChildMessage(const std::string& message) noexcept;

  asio::io_service& io_service_;
  /*! \internal
   *  \brief Socket used as communication channel from child to parent
   */
  std::shared_ptr<asio::ip::tcp::socket> socket_;
  /*! \internal
   *  \brief Short-lived acceptor used to establish the one connection from child to parent
   */
  std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;
  /*! \internal
   *  \brief All handlers for child to parent communication and the stop() happens on this strand
   */
  asio::io_service::strand strand_;

  asio::streambuf          buf_;

  int                      port_;

  std::string              sessionId_;
#ifndef WT_WIN32
  pid_t                    pid_;
#else // WT_WIN32
  PROCESS_INFORMATION      processInfo_;
#endif // WT_WIN32

  SessionProcessManager *manager_ = nullptr;

  std::function<void (bool)> listeningCallback_;
};

} // namespace server
} // namespace http

#endif // WT_SESSION_PROCESS_HPP
