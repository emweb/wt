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

  virtual void flush(ResponseState state, const WriteCallback& callback) override;
  virtual void readWebSocketMessage(const ReadCallback& callback) override;
  virtual bool webSocketMessagePending() const override;
  virtual bool detectDisconnect(const DisconnectCallback& callback) override;

  virtual std::istream& in() override { return reply_->in(); }
  virtual std::ostream& out() override { return reply_->out(); }
  virtual std::ostream& err() override { return std::cerr; }

  virtual void setStatus(int status) override;
  virtual void setContentLength(::int64_t length) override;

  virtual void addHeader(const std::string& name, const std::string& value) override;
  virtual void setContentType(const std::string& value) override;
  virtual void setRedirect(const std::string& url) override;

  virtual const char *contentType() const override;
  virtual ::int64_t contentLength() const override;

  virtual const char *envValue(const char *name) const override;
  virtual const char *headerValue(const char *name) const override;
  virtual std::vector<Wt::Http::Message::Header> headers() const override;
  virtual const std::string& serverName() const override;
  virtual const std::string& serverPort() const override;
  virtual const std::string& scriptName() const override;
  virtual const char *requestMethod() const override;
  virtual const std::string& queryString() const override;
  virtual const std::string& pathInfo() const override;
  virtual const std::string& remoteAddr() const override;
  virtual const char *urlScheme() const override;
  bool isSynchronous() const;
  virtual std::unique_ptr<Wt::WSslInfo> sslInfo(bool behindReverseProxy) const override;
  virtual const std::vector<std::pair<std::string, std::string> > &urlParams() const override;

private:
  WtReplyPtr reply_;
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
