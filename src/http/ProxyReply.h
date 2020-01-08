// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_PROXY_REPLY_HPP
#define HTTP_PROXY_REPLY_HPP

#include "Reply.h"
#include "SessionProcessManager.h"

namespace http {
namespace server {

class ProxyReply final : public Reply
{
public:
  ProxyReply(Request& request,
	     const Configuration& config,
	     SessionProcessManager& sessionManager);

  virtual ~ProxyReply();

  virtual void reset(const Wt::EntryPoint *ep) override;

  virtual void writeDone(bool success) override;

  virtual bool consumeData(const char *begin,
			   const char *end,
			   Request::State state) override;

  void closeClientSocket();

protected:
  virtual std::string contentType() override;
  virtual ::int64_t contentLength() override;

  virtual bool nextContentBuffers(std::vector<asio::const_buffer>& result) override;

private:
  void error(status_type status);

  std::string getSessionId() const;

  void connectToChild(bool success);
  void handleChildConnected(const Wt::AsioWrapper::error_code& ec);
  void assembleRequestHeaders();
  void handleDataWritten(const Wt::AsioWrapper::error_code& ec,
			 std::size_t transferred);
  void handleStatusRead(const Wt::AsioWrapper::error_code& ec);
  void handleHeadersRead(const Wt::AsioWrapper::error_code& ec);
  void handleResponseRead(const Wt::AsioWrapper::error_code& ec);

  void appendSSLInfo(const Wt::WSslInfo* sslInfo, std::ostream& os);

  bool sendReload();

  SessionProcessManager &sessionManager_;
  std::shared_ptr<SessionProcess> sessionProcess_;
  std::shared_ptr<asio::ip::tcp::socket> socket_;

  std::string contentType_;

  /// Request/response buffers for the child connection
  asio::streambuf requestBuf_;
  asio::streambuf responseBuf_;

  /// Response buffer
  asio::streambuf out_buf_;
  std::ostream out_;

  std::size_t sending_;
  ::int64_t contentLength_;
  bool more_;
  bool receiving_;
  bool fwCertificates_;

  const char *beginRequestBuf_;
  const char *endRequestBuf_;
  Request::State state_;

  Wt::Http::ParameterMap queryParams_;
};

} // namespace server
} // namespace http

#endif // HTTP_PROXY_REPLY_HPP
