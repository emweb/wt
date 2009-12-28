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

HTTPRequest::HTTPRequest(WtReplyPtr reply)
  : reply_(reply),
    instream_(reply_->cin())
{ }

void HTTPRequest::flush(ResponseState state, CallbackFunction callback,
			void *callbackData)
{
  reply_->setWaitMoreData(state == ResponseWaitMore);
  reply_->send(outstream_.str(), callback, callbackData);
  outstream_.str("");

  if (state == ResponseDone)
    delete this; // also deletes the reply if the connection was already closed
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
  Request::HeaderMap::const_iterator
    i = reply_->request().headerMap.find(name);
  if (i != reply_->request().headerMap.end())
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
  } else
    return std::string();
}

std::string HTTPRequest::serverName() const
{
  return Configuration::instance().serverName();
}

std::string HTTPRequest::serverPort() const
{
  return boost::lexical_cast<std::string>(reply_->request().port);
}

std::string HTTPRequest::scriptName() const
{
  return reply_->request().request_path;
}

std::string HTTPRequest::requestMethod() const
{
  return reply_->request().method;
}

std::string HTTPRequest::queryString() const
{
  return reply_->request().request_query;
}

std::string HTTPRequest::pathInfo() const
{
  return reply_->request().request_extra_path;
}

std::string HTTPRequest::remoteAddr() const
{
  return reply_->request().remoteIP;
}

std::string HTTPRequest::urlScheme() const
{
  return reply_->request().urlScheme;
}

} // namespace server
} // namespace http
