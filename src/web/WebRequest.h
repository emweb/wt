// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEB_REQUEST_H_
#define WEB_REQUEST_H_

#include <iostream>
#include <Wt/WDllDefs.h>
#include <Wt/WGlobal>
#include <Wt/Http/Request>

#include <boost/cstdint.hpp>
#include <boost/function.hpp>

namespace Wt {

class EntryPoint;

/*
 * A single, raw, HTTP request/response, which conveys all of the http-related
 * information to the application and gathers the response.
 */
class WT_API WebRequest
{
public:
  WebRequest();

  enum ResponseState {
    ResponseDone,
    ResponseFlush
  };

  enum ResponseType {
    Page,
    Script,
    Update
  };

  typedef boost::function<void(void)> CallbackFunction;

  void startAsync() { }

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
  virtual void flush(ResponseState state = ResponseDone,
		     CallbackFunction callback = CallbackFunction()) = 0;

  /*
   * For a web socket request (isWebSocketRequest()), read a message
   * and call the given callback function when done.
   *
   * The new message is available in in() and has length contentLength()
   */
  virtual void readWebSocketMessage(CallbackFunction callback);

  /*
   * For a web socket request (isWebSocketRequest()), returns whether
   * more data is available. This is used to defer a response but wait
   * for more incoming events.
   */
  virtual bool webSocketMessagePending() const;

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
  virtual void setContentLength(::int64_t length) = 0;

  /*
   * Adds a header for a normal response.
   */
  virtual void addHeader(const std::string& name, const std::string& value) = 0;

  /*
   * Returns request information, which are not http headers.
   */
  virtual std::string envValue(const std::string& name) const = 0;

  virtual std::string serverName() const = 0;
  virtual std::string serverPort() const = 0;
  virtual std::string scriptName() const = 0;
  virtual std::string requestMethod() const = 0;
  virtual std::string queryString() const = 0;
  virtual std::string pathInfo() const = 0;
  virtual std::string remoteAddr() const = 0;

  virtual std::string urlScheme() const = 0;

  virtual bool isWebSocketMessage() const {
    return false;
  }

  bool isWebSocketRequest() const {
    std::string s = urlScheme();
    return s == "ws" || s == "wss";
  }

  /*
   * Accesses to cgi environment variables and headers -- rfc2616 name 
   */
  virtual std::string headerValue(const std::string& name) const = 0;

  /*
   * Accesses to specific header fields (calls headerValue()).
   */
  std::string userAgent() const;
  std::string referer() const;

  /*
   * Accesses to specific information, which are not http headers
   * (calls envValue())
   */
  std::string contentType() const;
  int         contentLength() const;

#ifdef WT_TARGET_JAVA
  /*
   * In J2E, the servlet determines how session tracking is encoded in
   * the URL.
   */
  std::string encodeURL(const std::string& url) const;
#endif // WT_TARGET_JAVA

  const std::string *getParameter(const std::string& name) const;
  const Http::ParameterValues& getParameterValues(const std::string& name) const;
  const Http::ParameterMap& getParameterMap() const { return parameters_; }
  const Http::UploadedFileMap& uploadedFiles() const { return files_; }
  ::int64_t postDataExceeded() const { return postDataExceeded_; }

  WT_LOCALE parseLocale() const;

  void setResponseType(ResponseType responseType);
  ResponseType responseType() const { return responseType_; }

protected:
  const EntryPoint *entryPoint_;
  bool doingAsyncCallbacks_;

  void emulateAsync(ResponseState state);

  virtual ~WebRequest();

#ifndef WT_CNOR
  void setAsyncCallback(boost::function<void(void)> cb);
  boost::function<void(void)> getAsyncCallback();
#endif // WT_CNOR

private:
  std::string parsePreferredAcceptValue(const std::string& value) const;

  ::int64_t             postDataExceeded_;
  Http::ParameterMap    parameters_;
  Http::UploadedFileMap files_;
  ResponseType          responseType_;

  static Http::ParameterValues emptyValues_;

#ifndef WT_CNOR
  boost::function<void(void)> asyncCallback_;
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
