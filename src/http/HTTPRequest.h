// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HTTP_HTTP_REQUEST_H_
#define HTTP_HTTP_REQUEST_H_

#include <sstream>

#include "WebRequest.h"
#include "WtReply.h"
#include "Wt/Http/Message.h"

namespace http {
namespace server {

class HTTPRequest final : public Wt::WebResponse
{
public:
  HTTPRequest(WtReplyPtr wtReply, const Wt::EntryPoint *entryPoint);
  void reset(WtReplyPtr reply, const Wt::EntryPoint *entryPoint);
  bool done() const;

  void flush(ResponseState state, const WriteCallback& callback) override;
  void readWebSocketMessage(const ReadCallback& callback) override;
  bool webSocketMessagePending() const override;
  bool supportsTransferWebSocketResourceSocket() override { return true; }
  bool detectDisconnect(const DisconnectCallback& callback) override;

  std::istream& in() override { return reply_->in(); }
  std::ostream& out() override { return reply_->out(); }
  std::ostream& err() override { return std::cerr; }

  void setStatus(int status) override;
  int status() override;
  void setContentLength(::int64_t length) override;

  void addHeader(const std::string& name, const std::string& value) override;
  void insertHeader(const std::string& name, const std::string& value) override;
  void setContentType(const std::string& value) override;
  void setRedirect(const std::string& url) override;

  const char *contentType() const override;
  ::int64_t contentLength() const override;

  const char *envValue(const char *name) const override;
  const char *headerValue(const char *name) const override;
  std::vector<Wt::Http::Message::Header> headers() const override;
  const std::string& serverName() const override;
  const std::string& serverPort() const override;
  const std::string& scriptName() const override;
  const char *requestMethod() const override;
  const std::string& queryString() const override;
  const std::string& pathInfo() const override;
  const std::string& remoteAddr() const override;
  const char *urlScheme() const override;
  bool isSynchronous() const;
  std::unique_ptr<Wt::WSslInfo> sslInfo(const Wt::Configuration & conf) const override;
  const std::vector<std::pair<std::string, std::string> > &urlParams() const override;

private:
  WtReplyPtr reply_;
  // copy of what was written to reply_, because it is sometimes queried
  // after reply_ ptr is reset
  int status_;
  mutable std::string serverPort_;
  mutable std::vector<std::string> s_;

#ifdef HTTP_WITH_SSL
  // Extracts SSL info from internal Wt-specific base64-encoded JSON implementation,
  // used for Wt's own reverse proxy (dedicated session processes).
  std::unique_ptr<Wt::WSslInfo> sslInfoFromJson() const;
#endif // HTTP_WITH_SSL
  // Extract SSL info from X-SSL-Client-* headers. Can be used when Wt is behind an SSL-terminating
  // proxy like nginx or Apache (HAProxy's headers are not currently supported).
  std::unique_ptr<Wt::WSslInfo> sslInfoFromHeaders() const;

  const char *cstr(const buffer_string& bs) const;

  static const std::string empty_;
};

}
}

#endif // HTTP_HTTP_REQUEST_H_
