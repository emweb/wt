// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HTTP_CLIENT_H_
#define WT_HTTP_CLIENT_H_

#include <Wt/WFlags.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>

#include <Wt/Http/Message.h>
#include <Wt/Http/Method.h>

#include <Wt/AsioWrapper/namespace.hpp>
#include <Wt/AsioWrapper/system_error.hpp>

#include <chrono>
#include <string>

#ifndef WT_TARGET_JAVA
#ifdef WT_ASIO_IS_BOOST_ASIO
#include <boost/version.hpp>
namespace boost {
  namespace asio {
#if BOOST_VERSION >= 106600
    class io_context;
    typedef io_context io_service;
#else
    class io_service;
#endif
  }
}
#else // WT_ASIO_IS_STANDALONE_ASIO
#include <asio/version.hpp>
namespace asio {
#if ASIO_VERSION >= 101200
  class io_context;
  typedef io_context io_service;
#else
  class io_service;
#endif
}
#endif // WT_ASIO_IS_BOOST_ASIO
#endif // WT_TARGET_JAVA

namespace Wt {

#ifdef WT_TARGET_JAVA
  namespace AsioWrapper {
    namespace asio {
      struct io_service;
    }
  }
#endif // WT_TARGET_JAVA

  /*! \brief Namespace for \ref http handling
   */
  namespace Http {

/*! \defgroup http HTTP protocol (Wt::Http)
 *  \brief Classes that handle the HTTP protocol.
 *
 * The %Wt::%Http namespace contains classes that deal with the HTTP
 * protocol. This can be split in two groups:
 * - classes involved which deal with the server-side aspects of %Wt through WResource:
 *   - Request: a resource request
 *   - Response: a resource response
 *   - ResponseContinuation: an asynchronous response continuation object
 *   - UploadedFile: a file uploaded in the Request
 *
 * - classes that implement an HTTP client:
 *   - Client: an HTTP client
 *   - Message: a message to be sent with the client, or received from the client.
 */

/*! \class Client Wt/Http/Client.h Wt/Http/Client.h
 *  \brief An HTTP client.
 *
 * This class implements an HTTP client. It can be used to interact with
 * web services using the HTTP protocol.
 *
 * The client uses asynchronous I/O and only provides an asynchronous
 * interface: you cannot actively wait for an operation to complete,
 * instead the client will notify you of the result using the done()
 * signal.
 *
 * Because the client uses asynchronous I/O, it does its work within
 * the scope of an event-driven thread pool implementation. By
 * default, this is the same thread pool that is used by the %Wt
 * server, available through WServer::ioService(), but you may also
 * use the client by providing it an explicit I/O service to be used.
 *
 * The client supports the HTTP and HTTPS (if %Wt was built with
 * OpenSSL support) protocols, and can be used for GET and POST
 * methods. One client can do only one operation at a time.
 *
 * Usage example:
 * \code
 *    ...
 *    auto client = addChild(std::make_unique<Http::Client>());
 *    client->setTimeout(std::chrono::seconds{15});
 *    client->setMaximumResponseSize(10 * 1024);
 *    client->done().connect(std::bind(&MyWidget::handleHttpResponse, this, _1, _2));
 *    if (client->get("http://www.webtoolkit.eu/wt/blog/feed/"))
 *      WApplication::instance()->deferRendering();
 *    else {
 *      // in case of an error in the %URL
 *    }
 * }
 *
 * void handleHttpResponse(std::system::error_code err, const Http::Message& response)
 * {
 *    WApplication::instance()->resumeRendering();
 *
 *    if (!err && response.status() == 200) {
 *       ... parse response.body()
 *    }
 * }
 * \endcode
 *
 * The function connected to the done() signal will be run within the
 * context of the application that created the client. WServer::post()
 * is used for this.
 *
 * <h3>Basic access authentication</h3>
 * When you want to add authentication information in the %URL, this can be done
 * as <tt>https://username:password@www.example.com/</tt>. When doing this,
 * make sure that the username and password string are URL-encoded
 * (\ref Wt::Utils::urlEncode). For example,
 * <tt>https://username:pass word@www.example.com/</tt> should be passed as
 * <tt>https://username:pass%20word@www.example.com/</tt>.
 *
 * \ingroup http
 */
class WT_API Client : public WObject
{
public:
  /*! \brief Default constructor.
   *
   * The client uses the I/O service and thread-pool from the current
   * WApplication::instance().
   */
  Client();

  /*! \brief Constructor.
   *
   * The client uses the given I/O service and thread-pool, and is
   * useful to use the client outside the context of a web
   * application.
   */
  Client(Wt::AsioWrapper::asio::io_service& ioService);

  /*! \brief Destructor.
   *
   * If the client is still busy, the current request is aborted.
   *
   * \sa abort()
   */
  virtual ~Client();

  /*! \brief Sets an I/O timeout.
   *
   * This sets a timeout waiting for I/O operations. The timeout does
   * not bound the total timeout, since the timer is reset on each I/O
   * progress.
   *
   * The default timeout is 10 seconds.
   */
  void setTimeout(std::chrono::steady_clock::duration timeout);

  /*! \brief Returns the I/O timeout.
   *
   * \sa setTimeout()
   */
  std::chrono::steady_clock::duration timeout() const { return timeout_; }

  /*! \brief Sets a maximum response size.
   *
   * The response is stored in-memory. To avoid a DoS by a malicious
   * downstream HTTP server, the response size is bounded by an upper limit.
   *
   * The limit includes status line, headers and response body.
   *
   * The default value is 64 kilo bytes.
   *
   * A value of 0 has a special meaning: the size of the response will
   * not be limited, but the response body will not be stored in the
   * response message. Instead the data is made available only
   * to bodyDataReceived() to be processed incrementally.
   */
  void setMaximumResponseSize(std::size_t bytes);

  /*! \brief Returns the maximum response size.
   *
   * \sa setMaximumResponseSize()
   */
  std::size_t maximumResponseSize() const { return maximumResponseSize_; }

  /*! \brief Enables SSL certificate verification.
   *
   * For https requests, it is (very strongly!) recommended to perform
   * certificate verification: this verifies that you are indeed connected
   * to the server you intended (and not to a man-in-the-middle). Without
   * such a check, encryption simply isn't very useful.
   *
   * Nevertheless, there may be situations in which you will want to
   * disable this functionality.
   *
   * The default configuration is to have certificate verification
   * enabled.
   */
  void setSslCertificateVerificationEnabled(bool enabled);

  /*! \brief Returns whether SSL certificate verification is enabled.
   *
   * \sa setSslCertificateVerificationEnabled()
   */
  bool isSslCertificateVerificationEnabled() const { return verifyEnabled_; }

  /*! \brief Sets a SSL certificate used for server identity verification.
   *
   * This setting only affects a https request: it configures a certificate
   * file to be used to verify the identity of the server.
   */
  void setSslVerifyFile(const std::string& verifyFile);

  /*! \brief Sets a path with SSL certificates for server identity verification.
   *
   * This setting only affects a https request: it configures a
   * directory containing certificates to be used to verify the
   * identity of the server.
   */
  void setSslVerifyPath(const std::string& verifyPath);

  /*! \brief Starts a GET request.
   *
   * The function starts an asynchronous GET request, and returns
   * immediately.
   *
   * The function returns \c true when the GET request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \p false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * \sa request(), done()
   */
  bool get(const std::string& url);

  /*! \brief Starts a GET request.
   *
   * The function starts an asynchronous GET request, and returns
   * immediately.
   *
   * The function returns \c true when the GET request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \p false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * This function accepts one or more headers.
   *
   * \sa request(), done()
   */
  bool get(const std::string& url, const std::vector<Message::Header> headers);

  /*! \brief Starts a HEAD request.
   *
   * The function starts an asynchronous HEAD request, and returns
   * immediately.
   *
   * The function returns \c true when the HEAD request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \c false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * \sa request(), done()
   */
  bool head(const std::string &url);

  /*! \brief Starts a HEAD request.
   *
   * The function starts an asynchronous HEAD request, and returns
   * immediately.
   *
   * The function returns \c true when the HEAD request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \c false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * This function accepts one or more headers.
   *
   * \sa request(), done()
   */
  bool head(const std::string &url, const std::vector<Message::Header> headers);

  /*! \brief Starts a POST request.
   *
   * The function starts an asynchronous POST request, and returns
   * immediately.
   *
   * The function returns \c true when the POST request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \p false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * \sa request(), done()
   */
  bool post(const std::string& url, const Message& message);

  /*! \brief Starts a PUT request.
   *
   * The function starts an asynchronous PUT request, and returns
   * immediately.
   *
   * The function returns \c true when the PUT request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \p false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * \sa request(), done()
   */
  bool put(const std::string& url, const Message& message);
  
  /*! \brief Starts a DELETE request.
   *
   * The function starts an asynchronous DELETE request, and returns
   * immediately.
   *
   * The function returns \c true when the DELETE request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \p false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * \sa request(), done()
   */
  bool deleteRequest(const std::string& url, const Message& message);
  
  /*! \brief Starts a PATCH request.
   *
   * The function starts an asynchronous PATCH request, and returns
   * immediately.
   *
   * The function returns \c true when the PATCH request has been
   * scheduled, and thus done() will be emitted eventually.
   *
   * The function returns \p false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * \sa request(), done()
   */
  bool patch(const std::string& url, const Message& message);
  
  /*! \brief Starts a request.
   *
   * The function starts an asynchronous HTTP request, and returns
   * immediately.
   *
   * The function returns \c true when the request has been scheduled,
   * and thus done() will be emitted eventually.
   *
   * The function returns \p false if the client could not schedule
   * the request, for example if the \p url is invalid or if the %URL
   * scheme is not supported.
   *
   * \sa request(), done()
   */
  bool request(Http::Method method, const std::string& url,
	       const Message& message);

  /*! \brief Aborts the curent request.
   *
   * If the client is currently busy, this cancels the pending request.
   * done() will be emitted with asio::error::operation_aborted.
   *
   * \note The abort will be performed asynchronously, so it is possible
   *       that done() is still emitted with a successful response after
   *       abort() is called.
   */
  void abort();

  /*! \brief %Signal that is emitted when the current request is done.
   *
   * The \p error is 0 if the HTTP request was successful. Then, the
   * \p message contains the result.
   *
   * If the \p error is not 0, then an error message is given by
   * <tt>err.message()</tt>.
   *
   * Typical code to process the result is:
   * \code
   * void handle(Wt::AsioWrapper::error_code err, const Http::Message& response)
   * {
   *   if (!err) {
   *     if (response.status() == 200) {
   *       ... success
   *     }
   *   } else {
   *     Wt::log("error") << "Http::Client error: " << err.message();
   *   }
   * }
   * \endcode
   */
  Signal<Wt::AsioWrapper::error_code, Message>& done() {
    return done_;
  }

  /*! \brief %Signal that is emitted when all response headers have been
   *         received.
   *
   * The signal forwards the message with all headers, but with
   * empty body text. You may want to catch this signal if you want to
   * examine the headers prior to having received the complete message.
   *
   * \sa done(), bodyDataReceived()
   */
  Signal<Message>& headersReceived() {
    return headersReceived_;
  }

  /*! \brief %Signal that is emitted when more body data was received.
   *
   * The signal contains the next body data chunk received. You may
   * want to catch this signal if you want to process the response as it
   * is being received.
   *
   * You may want to use this in combination with
   * setMaximumResponseSize(0) to handle very long responses.
   */
  Signal<std::string>& bodyDataReceived() {
    return bodyDataReceived_;
  }

  /*! \brief Utility class representing an %URL.
   */
  struct URL {
    //! The protocol (e.g. "http")
    std::string protocol;
    //! Authentication
    std::string auth;
    //! The host (e.g. "www.webtoolkit.eu")
    std::string host;
    //! The port number (e.g. 80)
    int port;
    //! The path (e.g. "/wt")
    std::string path;
  };

  /*! \brief Utility method to parse a %URL.
   *
   * This parses a %URL into an URL object.
   *
   * The method returns true if the %URL could be parsed successfully.
   */
  static bool parseUrl(const std::string &url, URL &parsedUrl);

  /*! \brief Returns whether the client will follow redirects.
   *
   * \sa setFollowRedirect
   */
  bool followRedirect() const;

  /*! \brief Set whether the client will follow redirects.
   *
   * If set and the request method is GET, redirects are automatically
   * followed.
   */
  void setFollowRedirect(bool followRedirect);

  /*! \brief Returns the maximum number of redirects to follow.
   *
   * \sa setMaxRedirects()
   */
  int maxRedirects() const;

  /*! \brief Set the maximum number of redirects to follow.
   *
   * This is used only when followRedirect() is enabled.
   *
   * The default is 20.
   */
  void setMaxRedirects(int maxRedirects);

private:
  Wt::AsioWrapper::asio::io_service *ioService_;
  class Impl;
  std::shared_ptr<Impl> impl_;
  std::chrono::steady_clock::duration timeout_;
  std::size_t maximumResponseSize_;
  bool verifyEnabled_;
  std::string verifyFile_, verifyPath_;
  Signal<Wt::AsioWrapper::error_code, Message> done_;
  Signal<Message> headersReceived_;
  Signal<std::string> bodyDataReceived_;
  bool followRedirect_;
  int redirectCount_;
  int maxRedirects_;

  class TcpImpl;
  class SslImpl;

  void handleRedirect(Http::Method method, Wt::AsioWrapper::error_code err,
		      const Message& response, const Message& request);

  void emitDone(Wt::AsioWrapper::error_code err, const Message& response);
  void emitHeadersReceived(const Message& response);
  void emitBodyReceived(const std::string& data);
};

  }
}

#endif // WT_HTTP_CLIENT_H_
