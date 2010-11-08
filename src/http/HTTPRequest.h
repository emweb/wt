// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HTTP_HTTP_REQUEST_H_
#define HTTP_HTTP_REQUEST_H_

#include <sstream>

#include "WebRequest.h"
#include "WtReply.h"

namespace http {
namespace server {

class HTTPRequest : public Wt::WebRequest
{
public:
  HTTPRequest(WtReplyPtr wtReply, const Wt::EntryPoint *entryPoint);

  virtual void flush(ResponseState state, CallbackFunction callback);
  virtual void readWebSocketMessage(CallbackFunction callback);
  virtual bool webSocketMessagePending() const;

  virtual std::istream& in() { return reply_->cin(); }
  virtual std::ostream& out() { return outstream_; }
  virtual std::ostream& err() { return std::cerr; }

  virtual void setStatus(int status);
  virtual void setContentLength(::int64_t length);

  virtual void addHeader(const std::string& name, const std::string& value);
  virtual void setContentType(const std::string& value);
  virtual void setRedirect(const std::string& url);

  virtual std::string envValue(const std::string& name) const;
  virtual std::string headerValue(const std::string& name) const;
  virtual std::string serverName() const;
  virtual std::string serverPort() const;
  virtual std::string scriptName() const;
  virtual std::string requestMethod() const;
  virtual std::string queryString() const;
  virtual std::string pathInfo() const;
  virtual std::string remoteAddr() const;
  virtual std::string urlScheme() const;
  virtual bool isSynchronous() const;

private:
  WtReplyPtr reply_;
  std::stringstream outstream_;
};

}
}

#endif // HTTP_HTTP_REQUEST_H_
