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
  void reset(WtReplyPtr reply, const Wt::EntryPoint *entryPoint);
  bool done() const;

  virtual void flush(ResponseState state, const WriteCallback& callback);
  virtual void readWebSocketMessage(const ReadCallback& callback);
  virtual bool webSocketMessagePending() const;

  virtual std::istream& in() { return reply_->in(); }
  virtual std::ostream& out() { return reply_->out(); }
  virtual std::ostream& err() { return std::cerr; }

  virtual void setStatus(int status);
  virtual void setContentLength(::int64_t length);

  virtual void addHeader(const std::string& name, const std::string& value);
  virtual void setContentType(const std::string& value);
  virtual void setRedirect(const std::string& url);

  virtual const char *contentType() const;
  virtual ::int64_t contentLength() const;

  virtual const char *envValue(const char *name) const;
  virtual const char *headerValue(const char *name) const;
  virtual const std::string& serverName() const;
  virtual const std::string& serverPort() const;
  virtual const std::string& scriptName() const;
  virtual const char *requestMethod() const;
  virtual const std::string& queryString() const;
  virtual const std::string& pathInfo() const;
  virtual const std::string& remoteAddr() const;
  virtual const char *urlScheme() const;
  virtual bool isSynchronous() const;
  virtual Wt::WSslInfo *sslInfo() const;

private:
  WtReplyPtr reply_;
  mutable std::string serverPort_;
  mutable std::vector<std::string> s_;

  const char *cstr(const buffer_string& bs) const;

  static const std::string empty_;
};

}
}

#endif // HTTP_HTTP_REQUEST_H_
