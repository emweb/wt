// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2014 Emweb bvba, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_PROXY_REPLY_HPP
#define HTTP_PROXY_REPLY_HPP

#include "Reply.h"
#include "SessionProcessManager.h"

namespace http {
namespace server {

class ProxyReply : public Reply
{
public:
  ProxyReply(Request& request,
	     const Configuration& config,
	     SessionProcessManager& sessionManager);

  virtual ~ProxyReply();

  virtual void reset(const Wt::EntryPoint *ep);

  virtual void writeDone(bool success);

  virtual bool consumeData(Buffer::const_iterator begin,
			   Buffer::const_iterator end,
			   Request::State state);

protected:
  virtual std::string contentType();
  virtual ::int64_t contentLength();

  virtual bool nextContentBuffers(std::vector<asio::const_buffer>& result);

private:
  void error(status_type status);

  std::string getSessionId() const;

  void connectToChild(bool success);
  void handleChildConnected(const boost::system::error_code& ec);
  void assembleRequestHeaders();
  void handleDataWritten(const boost::system::error_code& ec,
			 std::size_t transferred);
  void handleStatusRead(const boost::system::error_code& ec);
  void handleHeadersRead(const boost::system::error_code& ec);
  void handleResponseRead(const boost::system::error_code& ec);

  SessionProcessManager &sessionManager_;
  boost::shared_ptr<SessionProcess> sessionProcess_;
  boost::shared_ptr<asio::ip::tcp::socket> socket_;

  std::string contentType_;

  /// Request/response buffers for the child connection
  asio::streambuf requestBuf_;
  asio::streambuf responseBuf_;

  /// Response buffer
  asio::streambuf out_buf_;
  std::ostream out_;

  std::size_t sending_;
  bool more_;
  bool receiving_;

  Buffer::const_iterator beginRequestBuf_;
  Buffer::const_iterator endRequestBuf_;
  Request::State state_;
};

} // namespace server
} // namespace http

#endif // HTTP_PROXY_REPLY_HPP
