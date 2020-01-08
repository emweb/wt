// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEB_SOCKET_MESSAGE_H_
#define WEB_SOCKET_MESSAGE_H_

#include "WebRequest.h"

namespace Wt {

/*
 * Wraps a WebSocket message as a web request.
 */
class WT_API WebSocketMessage final : public WebResponse
{
public:
  WebSocketMessage(WebSession *session);

  virtual void flush(ResponseState state = ResponseState::ResponseDone,
		     const WriteCallback& callback = WriteCallback()) override;

  void setWebSocketMessageCallback(const ReadCallback& callback);
  virtual bool webSocketMessagePending() const override;

  virtual std::istream& in() override;
  virtual std::ostream& out() override;
  virtual std::ostream& err() override;

  virtual void setRedirect(const std::string& url) override;
  virtual void setStatus(int status) override;
  virtual void setContentType(const std::string& value) override;
  virtual void setContentLength(::int64_t length) override;

  virtual void addHeader(const std::string& name, const std::string& value) override;
  virtual const char *envValue(const char *name) const override;

  virtual const std::string& serverName() const override;
  virtual const std::string& serverPort() const override;
  virtual const std::string& scriptName() const override;
  virtual const char *requestMethod() const override;
  virtual const std::string& queryString() const override;
  virtual const std::string& pathInfo() const override;
  virtual const std::string& remoteAddr() const override;

  virtual const char *urlScheme() const override;

  virtual std::unique_ptr<Wt::WSslInfo> sslInfo(bool behindReverseProxy) const override;

  virtual const char * headerValue(const char *name) const override;

#ifndef WT_TARGET_JAVA
  virtual std::vector<Wt::Http::Message::Header> headers() const override;
#endif

  virtual bool isWebSocketMessage() const override { return true; }

  virtual const char *contentType() const override;
  virtual ::int64_t contentLength() const override;

private:
  WebSession *session_;
  std::string queryString_;

  WebRequest *webSocket() const;
  void error(const std::string& msg) const;
};

}

#endif // WEB_SOCKET_MESSAGE_H_
