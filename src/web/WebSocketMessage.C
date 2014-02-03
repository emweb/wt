/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WLogger"

#include "WebSession.h"
#include "WebSocketMessage.h"

namespace Wt {

LOGGER("WebSocketMessage");

WebSocketMessage::WebSocketMessage(WebSession *session)
  : session_(session)
{ 
  queryString_ = "wtd=" + session_->sessionId() + "&request=jsupdate";
}

void WebSocketMessage::flush(ResponseState state,
			     const WriteCallback& callback)
{
  if (state != ResponseDone)
    error("flush(" + boost::lexical_cast<std::string>(state) + ") expected");

  session_->pushUpdates();

  delete this;
}

void WebSocketMessage::setWebSocketMessageCallback(const ReadCallback& callback)
{
  error("setWebSocketMessageCallback() not supported");
}

bool WebSocketMessage::webSocketMessagePending() const
{
  error("webSocketMessagePending() not supported");
  return false;
}

std::istream& WebSocketMessage::in()
{
  return webSocket()->in();
}

std::ostream& WebSocketMessage::out()
{
  return webSocket()->out();
}

std::ostream& WebSocketMessage::err()
{
  return webSocket()->err();
}

void WebSocketMessage::setRedirect(const std::string& url)
{
  error("setRedirect() not supported");
}

void WebSocketMessage::setStatus(int status)
{
  error("setStatus() not supported");
}

void WebSocketMessage::setContentType(const std::string& value)
{
  if (value != "text/javascript; charset=UTF-8")
    error("setContentType(): text/javascript expected");
}

void WebSocketMessage::setContentLength(::int64_t length)
{
  // We have no use for it, web socket messages are framed
}

void WebSocketMessage::addHeader(const std::string& name,
				 const std::string& value)
{
  if (name == "Set-Cookie")
    out() << "document.cookie=" << WWebWidget::jsStringLiteral(value) << ";";
}

const char *WebSocketMessage::envValue(const char *name) const
{
  return webSocket()->envValue(name);
}

::int64_t WebSocketMessage::contentLength() const
{
  webSocket()->in().seekg(0, std::ios::end);
  int length = webSocket()->in().tellg();
  webSocket()->in().seekg(0, std::ios::beg);
  return length;
}

const char *WebSocketMessage::contentType() const
{
  return "application/x-www-form-urlencoded";
}

const std::string& WebSocketMessage::serverName() const
{
  return webSocket()->serverName();
}

const std::string& WebSocketMessage::serverPort() const
{
  return webSocket()->serverPort();
}

const std::string& WebSocketMessage::scriptName() const
{
  return webSocket()->scriptName();
}

const char *WebSocketMessage::requestMethod() const
{
  return "POST";
}

const std::string& WebSocketMessage::queryString() const
{
  return queryString_;
}

const std::string& WebSocketMessage::pathInfo() const
{
  return webSocket()->pathInfo();
}

const std::string& WebSocketMessage::remoteAddr() const
{
  return webSocket()->remoteAddr();
}

const char *WebSocketMessage::urlScheme() const
{
  return "http";
}

Wt::WSslInfo *WebSocketMessage::sslInfo() const
{
  return webSocket()->sslInfo();
}

const char *WebSocketMessage::headerValue(const char *name) const
{
  return webSocket()->headerValue(name);
}

void WebSocketMessage::error(const std::string& msg) const
{
  LOG_ERROR("WebSocketMessage error: " + msg);
}

WebRequest *WebSocketMessage::webSocket() const
{
  return session_->asyncResponse_;
}

}
