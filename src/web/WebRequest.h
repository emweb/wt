// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEB_REQUEST_H_
#define WEB_REQUEST_H_

#include <chrono>
#include <cstdint>
#include <iostream>
#include <Wt/WDllDefs.h>
#include <Wt/WGlobal.h>
#include <Wt/Http/Request.h>

#include <cstdint>
#include <functional>

namespace Wt {

class EntryPoint;
class WSslInfo;

/*
 * A single, raw, HTTP request/response, which conveys all of the http-related
 * information to the application and gathers the response.
 */
class WT_API WebRequest
{
public:
  WebRequest();

  void log();

  enum class ResponseState {
    ResponseDone,
    ResponseFlush
  };

  enum class ResponseType {
    Page,
    Script,
    Update
  };

  typedef std::function<void(WebWriteEvent)> WriteCallback;
  typedef std::function<void(WebReadEvent)> ReadCallback;
  typedef std::function<void(void)> DisconnectCallback;

  /*
   * Signal that the response should be flushed.
   *
   * ResponseDone: flush & close
   *
   * ResponseFlush: flush what we have so far, do not close
   *  - callback must be specified for ResponseFlush, and is called
   *    if more data can be written. Until then, you cannot do new
   *    writes.
   */
  virtual void flush(ResponseState state = ResponseState::ResponseDone,
		     const WriteCallback& callback = WriteCallback()) = 0;
#ifdef WT_TARGET_JAVA
  virtual void flushBuffer();
#endif
  /*
   * For a web socket request (isWebSocketRequest()), read a message
   * and call the given callback function when done.
   *
   * The new message is available in in() and has length contentLength()
   */
  virtual void readWebSocketMessage(const ReadCallback& callback);

  /*
   * For a web socket request (isWebSocketRequest()), returns whether
   * more data is available. This is used to defer a response but wait
   * for more incoming events.
   */
  virtual bool webSocketMessagePending() const;

  /*
   * Indicate that we're deferring to write a response, but in the mean-time
   * we do want to know if there's a disconnect event (by reading from
   * the socket).
   */
  virtual bool detectDisconnect(const DisconnectCallback& callback);

  /*
   * Access the stream that contains the request body (HTTP) or a
   * single message (WS)
   */
  virtual std::istream& in() = 0;

  /*
   * Access the stream to submit the response.
   *
   * This is either the entire response body (HTTP), or a single response
   * message (WS)
   */
  virtual std::ostream& out() = 0;

  WT_BOSTREAM& bout() { return out(); }

  /*
   * (Not used)
   */
  virtual std::ostream& err() = 0;

  /*
   * Sets the redirect (instead of anything else).
   */
  virtual void setRedirect(const std::string& url) = 0;

  /*
   * Sets the status
   */
  virtual void setStatus(int status) = 0;

  /*
   * Sets the content-type for a normal response.
   */
  virtual void setContentType(const std::string& value) = 0;

  /*
   * Sets the content-length for a normal response.
   */
  virtual void setContentLength(std::int64_t length) = 0;

  /*
   * Adds a header for a normal response.
   */
  virtual void addHeader(const std::string& name, const std::string& value) = 0;

  /*
   * Returns request information, which are not http headers.
   */
  virtual const char *envValue(const char *name) const = 0;

  virtual const std::string& serverName() const = 0;
  virtual const std::string& serverPort() const = 0;
  virtual const std::string& scriptName() const = 0;
  virtual const char *requestMethod() const = 0;
  virtual const std::string& queryString() const = 0;
  virtual const std::string& pathInfo() const = 0;
  virtual const std::string& remoteAddr() const = 0;

  virtual const char *urlScheme() const = 0;

  virtual bool isWebSocketMessage() const { return false; }

  bool isWebSocketRequest() const { return webSocketRequest_; }
  void setWebSocketRequest(bool ws) { webSocketRequest_ = ws; }

  /*
   * Accesses to cgi environment variables and headers -- rfc2616 name 
   */
  virtual const char *headerValue(const char *name) const = 0;

  /*
   * Accesses to specific header fields (calls headerValue()).
   */
  const char *userAgent() const;
  const char *referer() const;

#ifndef WT_TARGET_JAVA
  virtual std::vector<Wt::Http::Message::Header> headers() const = 0;
#endif

  virtual const char *contentType() const;
  virtual ::int64_t contentLength() const;

#ifdef WT_TARGET_JAVA
  /*
   * In JavaEE, the servlet determines how session tracking is encoded in
   * the URL.
   */
  std::string encodeURL(const std::string& url) const;
#endif // WT_TARGET_JAVA

  const std::string *getParameter(const std::string& name) const;
  const Http::ParameterValues& getParameterValues(const std::string& name)
    const;
  const Http::ParameterMap& getParameterMap() const { return parameters_; }
  const Http::UploadedFileMap& uploadedFiles() const { return files_; }
  ::int64_t postDataExceeded() const { return postDataExceeded_; }

  WLocale parseLocale() const;

  void setResponseType(ResponseType responseType);
  ResponseType responseType() const { return responseType_; }

  /* 
   * Returns \c nullptr if the request does not have SSL client certificate
   * information.
   */
  virtual std::unique_ptr<WSslInfo> sslInfo(bool behindReverseProxy) const = 0;

  virtual const std::vector<std::pair<std::string, std::string> >& urlParams() const;

protected:
  const EntryPoint *entryPoint_;

  virtual ~WebRequest();
  void reset();

#ifndef WT_CNOR
  struct AsyncEmulation;
  AsyncEmulation *async_;

  void emulateAsync(ResponseState state);
  void setAsyncCallback(const WriteCallback& cb);
  const WriteCallback& getAsyncCallback();
#endif // WT_CNOR

private:
  std::string parsePreferredAcceptValue(const char *value) const;

  ::int64_t postDataExceeded_;
  Http::ParameterMap parameters_;
  Http::UploadedFileMap files_;
  ResponseType responseType_;
  bool webSocketRequest_;
  std::chrono::high_resolution_clock::time_point start_;
  std::vector<std::pair<std::string, std::string> > urlParams_;

  static Http::ParameterValues emptyValues_;

#ifndef WT_CNOR
  WriteCallback asyncCallback_;
#endif // WT_CNOR

  friend class CgiParser;
  friend class Http::Request;
  friend class WEnvironment;
  friend class WebController;
};

class WebResponse : public WebRequest
{
};

}

#endif // WEB_REQUEST_H_
