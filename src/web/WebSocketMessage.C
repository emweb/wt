/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "WebSession.h"
#include "WebSocketMessage.h"
#include "WtException.h"
#include "Utils.h"

namespace Wt {

WebSocketMessage::WebSocketMessage(WebSession *session)
  : session_(session)
{ }

void WebSocketMessage::flush(ResponseState state,
			     CallbackFunction callback)
{
  if (state != ResponseDone)
    error("flush(" + boost::lexical_cast<std::string>(state) + ") expected");

  session_->pushUpdates();

  delete this;
}

void WebSocketMessage::setWebSocketMessageCallback(CallbackFunction callback)
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

std::string WebSocketMessage::envValue(const std::string& name) const
{
  if (name == "CONTENT_LENGTH") {
    webSocket()->in().seekg(0, std::ios::end);
    int length = webSocket()->in().tellg();
    webSocket()->in().seekg(0, std::ios::beg);
    return boost::lexical_cast<std::string>(length);
  } else if (name == "CONTENT_TYPE")
    return "application/x-www-form-urlencoded";
  else  
    return webSocket()->envValue(name);
}

std::string WebSocketMessage::serverName() const
{
  return webSocket()->serverName();
}

std::string WebSocketMessage::serverPort() const
{
  return webSocket()->serverPort();
}

std::string WebSocketMessage::scriptName() const
{
  return webSocket()->scriptName();
}

std::string WebSocketMessage::requestMethod() const
{
  return "POST";
}

std::string WebSocketMessage::queryString() const
{
  return webSocket()->queryString() + "&request=jsupdate";
}

std::string WebSocketMessage::pathInfo() const
{
  return webSocket()->pathInfo();
}

std::string WebSocketMessage::remoteAddr() const
{
  return webSocket()->remoteAddr();
}

std::string WebSocketMessage::urlScheme() const
{
  return "http";
}

std::string WebSocketMessage::headerValue(const std::string& name) const
{
  return webSocket()->headerValue(name);
}

void WebSocketMessage::error(const std::string& msg) const
{
  throw WtException("WebSocketMessage error: " + msg);
}

WebRequest *WebSocketMessage::webSocket() const
{
  return session_->asyncResponse_;
}

}
