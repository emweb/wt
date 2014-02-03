// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
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
class WT_API WebSocketMessage : public WebRequest
{
public:
  WebSocketMessage(WebSession *session);

  virtual void flush(ResponseState state = ResponseDone,
		     const WriteCallback& callback = WriteCallback());

  virtual void setWebSocketMessageCallback(const ReadCallback& callback);
  virtual bool webSocketMessagePending() const;

  virtual std::istream& in();
  virtual std::ostream& out();
  virtual std::ostream& err();

  virtual void setRedirect(const std::string& url);
  virtual void setStatus(int status);
  virtual void setContentType(const std::string& value);
  virtual void setContentLength(::int64_t length);

  virtual void addHeader(const std::string& name, const std::string& value);
  virtual const char *envValue(const char *name) const;

  virtual const std::string& serverName() const;
  virtual const std::string& serverPort() const;
  virtual const std::string& scriptName() const;
  virtual const char *requestMethod() const;
  virtual const std::string& queryString() const;
  virtual const std::string& pathInfo() const;
  virtual const std::string& remoteAddr() const;

  virtual const char *urlScheme() const;

  virtual Wt::WSslInfo*sslInfo() const;

  virtual const char * headerValue(const char *name) const;

  virtual bool isWebSocketMessage() const { return true; }

  virtual const char *contentType() const;
  virtual ::int64_t contentLength() const;

private:
  WebSession *session_;
  std::string queryString_;

  WebRequest *webSocket() const;
  void error(const std::string& msg) const;
};

}

#endif // WEB_SOCKET_MESSAGE_H_
