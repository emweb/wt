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

#ifndef WT_WIN32
#include <sys/types.h>
#endif // WT_WIN32

namespace http {
namespace server {

namespace asio = Wt::AsioWrapper::asio;

class SessionProcess
  : public std::enable_shared_from_this<SessionProcess>
{
public:
  SessionProcess(asio::io_service &io_service);
  SessionProcess(const SessionProcess&) = delete;

  void stop();

  // Execute the session process, and call the onReady callback
  // function when done. The bool passed on to the onReady function
  // indicates success.
  void asyncExec(const Configuration &config,
		 const std::function<void (bool)>& onReady 
		   = std::function<void (bool)>());

  // Check whether the process is ready to accept connections
  bool ready() const { return port_ != -1; }
  int port() const { return port_; }
#ifndef WT_WIN32
  pid_t pid() const { return pid_; }
#else // WT_WIN32
  DWORD pid() const { return processInfo_.dwProcessId; }
  PROCESS_INFORMATION& processInfo() { return processInfo_; }
#endif // WT_WIN32

  const std::string& sessionId() const { return sessionId_; }
  void setSessionId(const std::string& sessionId);

  // Get the endpoint to connect to
  asio::ip::tcp::endpoint endpoint() const;

  void closeClientSocket();

private:
  void exec(const Configuration& config,
	    const std::function<void (bool)>& onReady);
  void acceptHandler(const Wt::AsioWrapper::error_code& err,
		     const std::function<void (bool)>& onReady);
  void readPortHandler(const Wt::AsioWrapper::error_code& err,
		       std::size_t transferred,
		       const std::function<void (bool)>& onReady);

  // Short-lived objects during startup
  asio::io_service& io_service_;
  std::shared_ptr<asio::ip::tcp::socket> socket_;
  std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;

  int			   port_;

  char			   buf_[6];

  std::string		   sessionId_;
#ifndef WT_WIN32
  pid_t			   pid_;
#else // WT_WIN32
  PROCESS_INFORMATION      processInfo_;
#endif // WT_WIN32
};

} // namespace server
} // namespace http

#endif // WT_SESSION_PROCESS_HPP
