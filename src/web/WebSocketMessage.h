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
		     CallbackFunction callback = CallbackFunction());

  virtual void setWebSocketMessageCallback(CallbackFunction callback);
  virtual bool webSocketMessagePending() const;

  virtual std::istream& in();
  virtual std::ostream& out();
  virtual std::ostream& err();

  virtual void setRedirect(const std::string& url);
  virtual void setStatus(int status);
  virtual void setContentType(const std::string& value);
  virtual void setContentLength(::int64_t length);

  virtual void addHeader(const std::string& name, const std::string& value);
  virtual std::string envValue(const std::string& name) const;

  virtual std::string serverName() const;
  virtual std::string serverPort() const;
  virtual std::string scriptName() const;
  virtual std::string requestMethod() const;
  virtual std::string queryString() const;
  virtual std::string pathInfo() const;
  virtual std::string remoteAddr() const;
  virtual std::string urlScheme() const;

  virtual std::string headerValue(const std::string& name) const;

  virtual bool isWebSocketMessage() const {
    return true;
  }

private:
  WebSession *session_;

  WebRequest *webSocket() const;
  void error(const std::string& msg) const;
};

}

#endif // WEB_SOCKET_MESSAGE_H_
