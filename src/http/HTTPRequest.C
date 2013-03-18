/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "HTTPRequest.h"
#include "Configuration.h"
#include "WtReply.h"

namespace http {
namespace server {

HTTPRequest::HTTPRequest(WtReplyPtr reply, const Wt::EntryPoint *entryPoint)
  : reply_(reply)
{
  entryPoint_ = entryPoint;
}

bool HTTPRequest::done() const
{
  return !reply_.get();
}

void HTTPRequest::flush(ResponseState state, const WriteCallback& callback)
{
  WtReplyPtr ptr = reply_;

  if (state == ResponseDone)
    reply_.reset();

  ptr->send(callback, state == ResponseDone);
}

void HTTPRequest::readWebSocketMessage(const ReadCallback& callback)
{
  reply_->readWebSocketMessage(callback);
}

bool HTTPRequest::webSocketMessagePending() const
{
  return reply_->readAvailable();
}

void HTTPRequest::setStatus(int status)
{
  reply_->setStatus((Reply::status_type) status);
}

void HTTPRequest::setContentLength(::int64_t length)
{
  reply_->setContentLength(length);
}

void HTTPRequest::addHeader(const std::string& name, const std::string& value)
{
  reply_->addHeader(name, value);
}

void HTTPRequest::setContentType(const std::string& value)
{
  reply_->setContentType(value);
}

void HTTPRequest::setRedirect(const std::string& url)
{
  reply_->setLocation(url);
}

std::string HTTPRequest::headerValue(const std::string& name) const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  Request::HeaderMap::const_iterator i = p->request().headerMap.find(name);
  if (i != p->request().headerMap.end())
    return i->second;
  else
    return std::string();
}

std::string HTTPRequest::envValue(const std::string& name) const
{
  if (name == "CONTENT_TYPE") {
    return headerValue("Content-Type");
  } else if (name == "CONTENT_LENGTH") {
    return headerValue("Content-Length");
  } else if (name == "SERVER_SIGNATURE") {
    return "<address>Wt httpd Server ("
      + envValue("SERVER_SOFTWARE")
      + ")</address>";
  } else if (name == "SERVER_SOFTWARE") {
    return "Wthttpd/"
      + boost::lexical_cast<std::string>(WT_SERIES) + '.'
      + boost::lexical_cast<std::string>(WT_MAJOR) + '.'
      + boost::lexical_cast<std::string>(WT_MINOR);
  } else if (name == "SERVER_ADMIN") {
    return "webmaster@localhost"; // FIXME
  } else if (name == "REMOTE_ADDR") {
    return remoteAddr();
  } else if (name == "DOCUMENT_ROOT") {
    return reply_->configuration().docRoot();
  } else
    return std::string();
}

std::string HTTPRequest::serverName() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return p->configuration().serverName();
}

std::string HTTPRequest::serverPort() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return boost::lexical_cast<std::string>(p->request().port);
}

std::string HTTPRequest::scriptName() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return p->request().request_path;
}

std::string HTTPRequest::requestMethod() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return p->request().method;
}

std::string HTTPRequest::queryString() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return p->request().request_query;
}

std::string HTTPRequest::pathInfo() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return p->request().request_extra_path;
}

std::string HTTPRequest::remoteAddr() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return p->request().remoteIP;
}

std::string HTTPRequest::urlScheme() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return std::string();

  return p->urlScheme();
}

bool HTTPRequest::isSynchronous() const
{
  return false;
}

Wt::WSslInfo *HTTPRequest::sslInfo() const
{
  return reply_->request().sslInfo();
}

} // namespace server
} // namespace http
