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

const std::string HTTPRequest::empty_;

HTTPRequest::HTTPRequest(WtReplyPtr reply, const Wt::EntryPoint *entryPoint)
  : reply_(reply)
{
  entryPoint_ = entryPoint;
}

void HTTPRequest::reset(WtReplyPtr reply, const Wt::EntryPoint *entryPoint)
{
  WebRequest::reset();

  reply_ = reply;
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

const char *HTTPRequest::headerValue(const char *name) const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return 0;

  const Request::Header *i = p->request().getHeader(name);
  if (i)
    return cstr(i->value);
  else
    return 0;
}

const char *HTTPRequest::cstr(const buffer_string& bs) const {
  if (!bs.next)
    return bs.data;
  else {
    s_.push_back(bs.str());
    return s_.back().c_str();
  }
}

::int64_t HTTPRequest::contentLength() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return 0;

  return p->request().contentLength;
}

const char *HTTPRequest::contentType() const
{
  return headerValue("Content-Type");
}

const char *HTTPRequest::envValue(const char *name) const
{
  if (strcmp(name, "CONTENT_TYPE") == 0) {
    return headerValue("Content-Type");
  } else if (strcmp(name, "CONTENT_LENGTH") == 0) {
    return headerValue("Content-Length");
  } else if (strcmp(name, "SERVER_SIGNATURE") == 0) {
    return "<address>Wt httpd server</address>";
  } else if (strcmp(name, "SERVER_SOFTWARE") == 0) {
    return "Wthttpd/" WT_VERSION_STR ;
  } else if (strcmp(name, "SERVER_ADMIN") == 0) {
    return "webmaster@localhost";
  } else if (strcmp(name, "REMOTE_ADDR") == 0) {
    return remoteAddr().c_str();
  } else if (strcmp(name, "DOCUMENT_ROOT") == 0) {
    return reply_->configuration().docRoot().c_str();
  } else
    return 0;
}

const std::string& HTTPRequest::serverName() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->configuration().serverName();
}

const std::string& HTTPRequest::serverPort() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  if (serverPort_.empty())
    serverPort_ = boost::lexical_cast<std::string>(p->request().port);

  return serverPort_;
}

const std::string& HTTPRequest::scriptName() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().request_path;
}

const char * HTTPRequest::requestMethod() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return 0;

  return cstr(p->request().method);
}

const std::string& HTTPRequest::queryString() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().request_query;
}

const std::string& HTTPRequest::pathInfo() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().request_extra_path;
}

const std::string& HTTPRequest::remoteAddr() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().remoteIP;
}

const char *HTTPRequest::urlScheme() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return "http";

  return p->request().urlScheme;
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
