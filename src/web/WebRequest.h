// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEB_REQUEST_H_
#define WEB_REQUEST_H_

#include <iostream>

#include "Wt/WDllDefs.h"

namespace Wt {

/*
 * A single (http) request, which conveys all of the http-related information
 * to the application: the HTTP method, headers and the request body (in in()).
 *
 * When the application has finished the request, it must:
 * - set headers, response content type and stream the response to out()
 * - or, set a redirect.
 */
class WT_API WebRequest
{
public:
  WebRequest();

  /*
   * Signal that the request should be flushed.
   * Unless keepConnectionOpen() is set, the response is transmitted, and
   * the request is deleted.
   */
  virtual void flush();

  /*
   * Access the stream that contains the request body.
   */
  virtual std::istream& in() = 0;

  /*
   * Access the stream to submit the response.
   */
  virtual std::ostream& out() = 0;

  /*
   * (Not used)
   */
  virtual std::ostream& err() = 0;

  /*
   * Set the redirect (instead of anything else).
   */
  virtual void setRedirect(const std::string& url) = 0;

  /*
   * Set the content-type for a normal response.
   */
  virtual void setContentType(const std::string& value) = 0;

  /*
   * Add a header for a normal response.
   */
  virtual void addHeader(const std::string& name, const std::string& value) = 0;

  /*
   * Return request information, which are not http headers.
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

  /*
   * Access to cgi environment variables and headers -- rfc2616 name 
   */
  virtual std::string headerValue(const std::string& name) const = 0;

  /*
   * Synchronous use for long-lived connections.
   */
  void setKeepConnectionOpen(bool how);
  bool keepConnectionOpen() const { return keepConnectionOpen_; }

  /*
   * Access to specific header fields (calls headerValue()).
   */
  std::string userAgent() const;
  std::string referer() const;

  /*
   * Access to specific information, which are not http headers
   * (calls envValue())
   */
  std::string contentType() const;
  int         contentLength() const;

  void setId(int id);
  int id() const { return id_; }

private:
  bool keepConnectionOpen_;
  int id_;

protected:
  virtual ~WebRequest();
};

}

#endif // WEB_REQUEST_H_
