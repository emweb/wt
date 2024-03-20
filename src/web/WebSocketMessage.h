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

  void flush(ResponseState state = ResponseState::ResponseDone,
      const WriteCallback& callback = WriteCallback()) override;

  bool supportsTransferWebSocketResourceSocket() override { return false; }

  void setWebSocketMessageCallback(const ReadCallback& callback);
  bool webSocketMessagePending() const override;

  std::istream& in() override;
  std::ostream& out() override;
  std::ostream& err() override;

  void setRedirect(const std::string& url) override;
  void setStatus(int status) override;
  int status() override;
  void setContentType(const std::string& value) override;
  void setContentLength(::int64_t length) override;

  void addHeader(const std::string& name, const std::string& value) override;
  void insertHeader(const std::string& name, const std::string& value) override;
  const char *envValue(const char *name) const override;

  const std::string& serverName() const override;
  const std::string& serverPort() const override;
  const std::string& scriptName() const override;
  const char *requestMethod() const override;
  const std::string& queryString() const override;
  const std::string& pathInfo() const override;
  const std::string& remoteAddr() const override;

  const char *urlScheme() const override;

  std::unique_ptr<Wt::WSslInfo> sslInfo(const Configuration & conf) const override;

  const char * headerValue(const char *name) const override;

#ifndef WT_TARGET_JAVA
  std::vector<Wt::Http::Message::Header> headers() const override;
#endif

  bool isWebSocketMessage() const override { return true; }

  const char *contentType() const override;
  ::int64_t contentLength() const override;

private:
  WebSession *session_;
  std::string queryString_;

  WebRequest *webSocket() const;
  void error(const std::string& msg) const;
};

}

#endif // WEB_SOCKET_MESSAGE_H_
