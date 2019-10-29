// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <map>
#include <memory>
#include <string>
#include <istream>
#include <sstream>
#include <vector>

#include <Wt/Http/Message.h>

#include <Wt/WDllDefs.h>

namespace Wt {

class WResource;
class WSslInfo;
class WebRequest;
class WebSession;

  namespace Http {

/*! \class UploadedFile Wt/Http/Request.h Wt/Http/Request.h
 *  \brief Details about a file uploaded with a request to a resource.
 * 
 * \if cpp
 * \sa Request::uploadedFiles()
 * \endif
 *
 * \sa WResource::handleRequest()
 *
 * \ingroup http
 */
class WT_API UploadedFile {
public:
  UploadedFile();
  
  UploadedFile(const std::string& spoolFileName,
	       const std::string& clientFileName,
	       const std::string& contentType);

  /*! \brief Return the spool file name.
   *
   * This is the location on the local (server) filesystem where the uploaded
   * file is temporarily stored. Unless you call stealSpoolFile(), this file
   * is deleted automatically.
   */
  const std::string& spoolFileName() const;

  /*! \brief Returns the client file name.
   *
   * This is the location that was indicated by the browser. Depending on
   * the browser this is an absolute path or only the file name.
   */
  const std::string& clientFileName() const;

  /*! \brief Returns the file content type.
   *
   * Returns the content mime-type that was sent along with the uploaded
   * file.
   */
  const std::string& contentType() const;

  /*! \brief Steals the uploaded spool file.
   *
   * By stealing the spooled file, it is no longer automatically deleted
   * by %Wt.
   */
  void stealSpoolFile() const;

private:
  struct Impl {
    std::string spoolFileName, clientFileName, contentType;
    bool        isStolen;

    ~Impl();
    void cleanup();
  };

  std::shared_ptr<Impl> fileInfo_;
};


/*! \brief A list of parameter values.
 *
 * This is the type used to aggregate all values for a single parameter.
 */
#ifndef WT_TARGET_JAVA
typedef std::vector<std::string> ParameterValues;
#else
typedef std::string ParameterValues[];
#endif

/*! \brief A parameter value map.
 *
 * This is the type used aggregate plain parameter values in a request.
 */
typedef std::map<std::string, ParameterValues> ParameterMap;

extern const std::string *get(const ParameterMap& map,
			      const std::string& name);

/*! \brief A file parameter map.
 *
 * This is the type used aggregate file parameter values in a request.
 */
typedef std::multimap<std::string, UploadedFile> UploadedFileMap;

class ResponseContinuation;

/*! \class Request Wt/Http/Request.h Wt/Http/Request.h
 *  \brief A resource request.
 *
 * The request provides information of parameters, including uploaded
 * files, that were present in a request to a WResource.
 *
 * \sa WResource::handleRequest()
 *
 * \ingroup http
 */
class WT_API Request
{
public:
  /*! \brief A single byte range.
   */
  class WT_API ByteRange
  {
  public:
    /*! \brief Creates a (0,0) byteranges */
    ByteRange();

    /*! \brief Creates a byte range.
     */
    ByteRange(::uint64_t first, ::uint64_t last);

    /*! \brief Returns the first byte of this range.
     */
    ::uint64_t firstByte() const { return firstByte_; }

    /*! \brief Returns the last byte of this range.
     */
    ::uint64_t lastByte() const { return lastByte_; }

  private:
    ::uint64_t firstByte_, lastByte_;
  };

  /*! \brief A byte range specifier.
   *
   * \sa getRanges()
   */
  class WT_API ByteRangeSpecifier : public std::vector<ByteRange>
  {
  public:
    /*! \brief Creates an empty byte range specifier.
     *
     * The specifier is satisfiable but empty, indicating that no
     * ranges were present.
     */
    ByteRangeSpecifier();

    /*! \brief Returns whether the range is satisfiable.
     *
     * If the range specification is not satisfiable, RFC 2616 states you
     * should return a response status of 416. isSatisfiable() will return
     * true if a Range header was missing or a syntax error occured, in
     * which case the number of ByteRanges will be zero and the client
     * must send the entire file.
     */
    bool isSatisfiable() const { return satisfiable_; }

    /*! \brief Sets whether the specifier is satisfiable.
     */
    void setSatisfiable(bool satisfiable) { satisfiable_ = satisfiable; }

  private:
    bool satisfiable_;
  };

  /*! \brief Cookie map type.
   *
   * A map which associates a cookie name with a cookie value.
   *
   * \sa cookies()
   */
  typedef std::map<std::string, std::string> CookieMap;

  /*! \brief Returns the query parameters.
   *
   * Returns parameters that were passed to the query, either inside
   * the URL, or inside a POST request, excluding uploaded files.
   *
   * \sa uploadedFiles()
   */
  const ParameterMap& getParameterMap() const { return parameters_; }

  /*! \brief Returns uploaded file parameters.
   *
   * \sa getParameterMap()
   */
  const UploadedFileMap& uploadedFiles() const { return files_; }

  /*! \brief Returns all values for a query parameter.
   *
   * Returns all values defined for a parameter named \p name. A
   * single parameter may have multiple values, e.g. in the query
   * string '?param=value1&param=value2'.
   *
   * Returns an empty list if the query parameter does not exist.
   */
  const ParameterValues& getParameterValues(const std::string& name) const;

  /*! \brief Returns a query parameter value.
   *
   * Returns the first value defined for a parameter named \p name
   * or \c nullptr if the paramter does not exist.
   */
  const std::string *getParameter(const std::string& name) const;

  /*! \brief Returns an uploaded file
   *
   * Returns the file uploaded for a parameter named \p name or \c
   * nullptr if the parameter does not contain does not exist or was
   * not associated with a file input field.
   */
  const UploadedFile *getUploadedFile(const std::string& name) const;

  /*! \brief Returns a non-zero value that exceeded the maximum allowed request.
   *
   * \sa WApplication::requestTooLarge
   */
  ::int64_t tooLarge() const;

  /*! \brief Returns a continuation object.
   *
   * Returns a non-zero continuation object if the request is a continuation
   * request for an earlier response for which a continuation was created.
   *
   * \sa Response::createContinuation()
   */
  ResponseContinuation *continuation() const { return continuation_; }

  /*! \brief Returns the (public) server name.
   *
   * Returns the public server name. This is the server name that is
   * advertised to outside, which is determined in a OS specific
   * way.
   *
   * \sa serverPort()
   */
  std::string serverName() const;

  /*! \brief Returns the server port.
   *
   * Returns the server port number through which this request was received.
   *
   * \sa serverName()
   */
  std::string serverPort() const;

  /*! \brief Returns the request path.
   *
   * Returns the path at which this request was received (excluding internal
   * path information): it is the path at which the application or resource
   * is deployed. 
   *
   * \sa pathInfo()
   */
  std::string path() const;

  /*! \brief Returns the request path info.
   *
   * Returns additional path information internal to the path().
   *
   * \sa pathInfo()
   */
  std::string pathInfo() const;

  /*! \brief Returns the request query string.
   */
  std::string queryString() const;

  /*! \brief Returns the url scheme used.
   *
   * This is either <tt>"http"</tt> or <tt>"https"</tt>
   */
  std::string urlScheme() const;

  /*! \brief Returns the input stream for parsing the body.
   *
   * If the request was a POST with as contentType()
   * "application/x-www-form-urlencoded" or "multipart/form-data", the
   * input stream will already have been consumed by Wt's CGI parser,
   * and made available as parameters in the request.
   */
  std::istream& in() const;

  /*! \brief Returns the "Content Type" of the request body.
   *
   * \sa in()
   */
  std::string contentType() const;

  /*! \brief Returns the "Content Length" of the request body.
   *
   * \sa in()
   */
  int contentLength() const;

  /*! \brief Returns the user agent.
   *
   * The user agent, as reported in the HTTP <tt>User-Agent</tt> field.
   */
  std::string userAgent() const;

  /*! \brief Returns the IP address of the client.
   *
   * The (most likely) IP address of the client that is connected to
   * this session.
   *
   * This is taken to be the first public address that is given in the
   * Client-IP header, or in the X-Forwarded-For header (in case the
   * client is behind a proxy). If none of these headers is present,
   * the remote socket IP address is used. 
   */
  std::string clientAddress() const;

  /*! \brief Returns the cookies.
   *
   * This returns all cookies set for this request.
   *
   * Not all clients may support cookies or have cookies enabled.
   *
   * \sa getCookieValue()
   */
  const CookieMap& cookies() const { return cookies_; }

  /*! \brief Returns a cookie value.
   *
   * Returns \c nullptr if no value was set for the given cookie.
   *
   * \sa cookies()
   */
  const std::string *getCookieValue(const std::string& cookieName) const;

  /*! \brief Returns a header value.
   *
   * Returns an empty string if the header was not present.
   */
  std::string headerValue(const std::string& field) const;

#ifndef WT_TARGET_JAVA
  /*! \brief Returns all headers.
   * 
   * \note Use headerValue() if you need to know the value of certain known headers.
   *       This method is not written to be efficient, but can be useful for debugging.
   */
  std::vector<Message::Header> headers() const;
#endif

  /*! \brief Returns a raw CGI environment variable.
   *
   * Retrieves the value for the given CGI environment variable (like
   * <tt>"SSL_CLIENT_S_DN_CN"</tt>), if it is defined, otherwise an
   * empty string.
   */
  std::string getCgiValue(const std::string& varName) const;

  /*! \brief Returns the request method.
   *
   * Returns the HTTP request method ("GET", "POST", or other).
   */
  std::string method() const;

  /*! \brief Returns the requested ranges as in the HTTP Range header
   *
   * The filesize is used to adapt the ranges to the actual file size
   * as per rules of RFC 2616. If the file size is unknown, pass -1.
   *
   * You should check if the ranges are satisfiable using
   * ByteRangeSpecifier::isSatisfiable().
   */
   ByteRangeSpecifier getRanges(::int64_t filesize) const;

  /*! \brief Returns information on the SSL client certificate or \c
   * nullptr if no authentication took place.
   *
   * This function will return \c nullptr if no verification took
   * place, %Wt was compiled without SSL support, or the web server
   * was configured without client SSL certificates.
   *
   * This method may return a pointer to a WSslInfo object, while the
   * authentication may have failed. This depends on the configuration
   * of the web server. It is therefore important to always check the
   * verification result with WSslInfo::clientVerificationResult().
   *
   * Session-bound resources will probably not use this method, but rely on
   * the validation done at the start of the session (see sslInfo() in
   * WEnvironment). Static resources on the other hand don't have an
   * associated session, so using this method you can perform client
   * authentication verification.
   *
   * The object returned is owned by Request and will be deleted
   * when the Request object is destroyed.
   *
   * \includedoc ssl_client_headers.dox
   *
   * \sa WEnvironment::sslInfo()
   */
  WSslInfo *sslInfo() const;

#ifndef WT_TARGET_JAVA
  /*! \brief Get the value for the given URL parameter
   *
   * Example:
   *
   * If a static resource is deployed at /tags/${tag},
   * then, if a request is made for /tags/Wt, urlParam("tag")
   * will return "Wt".
   *
   * If the given parameter is not available, an empty string is returned.
   */
  std::string urlParam(const std::string &param) const;

  /*! \brief Get all URL parameters
   *
   * \sa urlParam()
   */
  const std::vector<std::pair<std::string, std::string> > &urlParams() const;
#endif // WT_TARGET_JAVA

  static ByteRangeSpecifier getRanges(const std::string &header,
				      ::int64_t filesize);

  static void parseFormUrlEncoded(const std::string& s,
				  ParameterMap& parameters);

  static void parseCookies(const std::string& cookie,
			   std::map<std::string, std::string>& result);

private:
  const WebRequest *request_;
  const ParameterMap& parameters_;
  const UploadedFileMap& files_;
  ResponseContinuation *continuation_;
  std::map<std::string, std::string> cookies_;
  mutable std::unique_ptr<WSslInfo> sslInfo_;

  Request(const WebRequest& request, ResponseContinuation *continuation);
  Request(const ParameterMap& parameters, const UploadedFileMap& files);
  ~Request();

  friend class Wt::WResource;
  friend class Wt::WebSession;
  friend class Wt::Http::ResponseContinuation;
};

  }
}

#endif // HTTP_REQUEST_H_
