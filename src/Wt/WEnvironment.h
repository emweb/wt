// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WENVIRONMENT_H_
#define WENVIRONMENT_H_

#include <chrono>
#include <string>
#include <map>
#include <vector>

#include <Wt/WDllDefs.h>
#include <Wt/Http/Request.h>
#include <Wt/WLocale.h>
#include <Wt/WSignal.h>

namespace Wt {

/*! \brief An enumeration type for specific user agent.
 *
 * \sa agent()
 */
enum class UserAgent {
  Unknown = 0,                //!< Unknown user agent
  IEMobile = 1000,            //!< Internet Explorer Mobile, 5 or older
  IE6 = 1001,                 //!< Internet Explorer 6
  IE7 = 1002,                 //!< Internet Explorer 7
  IE8 = 1003,                 //!< Internet Explorer 8
  IE9 = 1004,                 //!< Internet Explorer 9
  IE10 = 1005,                //!< Internet Explorer 10
  IE11 = 1006,                //!< Internet Explorer 11
  Edge = 1100,		      //!< Edge
  Opera = 3000,               //!< Opera
  Opera10 = 3010,             //!< Opera 10 or later
  WebKit = 4000,              //!< WebKit
  Safari = 4100,              //!< Safari 2 or older
  Safari3 = 4103,             //!< Safari 3
  Safari4 = 4104,             //!< Safari 4 or later
  Chrome0 = 4200,             //!< Chrome 0
  Chrome1 = 4201,             //!< Chrome 1
  Chrome2 = 4202,             //!< Chrome 2
  Chrome3 = 4203,             //!< Chrome 3
  Chrome4 = 4204,             //!< Chrome 4
  Chrome5 = 4205,             //!< Chrome 5 or later
  Arora = 4300,               //!< Arora
  MobileWebKit = 4400,        //!< Mobile WebKit
  MobileWebKitiPhone = 4450,  //!< Mobile WebKit iPhone/iPad
  MobileWebKitAndroid = 4500, //!< Mobile WebKit Android
  Konqueror = 5000,           //!< Konqueror
  Gecko = 6000,               //!< Gecko
  Firefox = 6100,             //!< Firefox 2 or older
  Firefox3_0 = 6101,          //!< Firefox 3.0
  Firefox3_1 = 6102,          //!< Firefox 3.1
  Firefox3_1b = 6103,         //!< Firefox 3.1b
  Firefox3_5 = 6104,          //!< Firefox 3.5
  Firefox3_6 = 6105,          //!< Firefox 3.6
  Firefox4_0 = 6106,          //!< Firefox 4.0
  Firefox5_0 = 6107,          //!< Firefox 5.0 or later
  BotAgent = 10000            //!< Bot user agent
};

/*! \brief Enumeration for HTML content type.
 */
enum class HtmlContentType {
  XHTML1, //!< XHTML1.x
  HTML4,  //!< HTML4
  HTML5   //!< HTML5
}; 

class WSslInfo;
class WAbstractServer;
class Configuration;
class WServer;
class WebRequest;
class WebSession;

/*! \class WEnvironment Wt/WEnvironment.h Wt/WEnvironment.h
 *  \brief A class that captures information on the application environment.
 *
 * The environment provides information on the client, and gives access
 * to startup arguments.
 *
 * Usage example:
 * \if cpp
 * \code
 * const WEnvironment& env = WApplication::instance()->environment();
 *
 * // read an application startup argument 
 * // (passed as argument in the URL or POST'ed to the application).
 * if (!env.getParameterValues("login").empty()) {
 *   std::string login = env.getParameterValues("login")[0];
 *   ...
 * }
 *
 * // Check for JavaScript/AJAX availability before using AJAX-only
 * // widgets
 * std::unique_ptr<Wt::WTextArea> textEdit;
 * if (!env.ajax())
 *   textEdit = std::make_unique<Wt::WTextEdit>(); // provide an HTML text editor
 * else
 *   textEdit = std::make_unique<Wt::WTextArea>(); // fall-back to a plain old text area.
 *
 * \endcode
 * \elseif java
 * \code
 * WEnvironment env = WApplication.instance().environment();
 *	 
 * // read an application startup argument 
 * // (passed as argument in the URL or POST'ed to the application).
 * if (!env.getParameterValues("login").isEmpty()) {
 * String login = env.getParameterValues("login").get(0);
 * //...
 * }
 *	 
 * // Check for JavaScript/AJAX availability before using JavaScript-only
 * // widgets
 *  WTextArea textEdit;
 * if (!env.isAjax())
 *   textEdit = new WTextEdit(); // provide an HTML text editor
 * else
 *   textEdit = new WTextArea(); // fall-back to a plain old text area.
 * \endcode
 * \endif
 */
class WT_API WEnvironment
{
public:
  /*! \brief Cookie map.
   *
   * A std::map which associates a cookie name with a cookie value.
   *
   * \sa cookies()
   */
  typedef std::map<std::string, std::string> CookieMap;

#ifdef WT_TARGET_JAVA
  /*! \brief %Wt's JavaScript scope.
   */
  static std::string javaScriptWtScope() { return WT_CLASS; } 
#endif //WT_TARGET_JAVA

  /*! \brief Parameters passed to the application.
   *
   * Arguments passed to the application, either in the URL for a
   * http GET, or in both the URL and data submitted in a http POST.
   *
   * \if cpp
   * \note The parameter map will be updated when the page is refreshed
   * (e.g. in non-Ajax sessions and when reload-is-new-session is false),
   * so you will have to make a copy of values in the parameter map if you want
   * to preserve them across different requests.
   * \endif
   *
   * \sa getParameterValues()
   */
  const Http::ParameterMap& getParameterMap() const { return parameters_; }

  /*! \brief Returns values for a query parameter.
   *
   * Returns an empty list if the parameter was not defined.
   *
   * One or more values may be associated with a single argument.
   *
   * For example a %Wt application <tt>foo.wt</tt> started as
   * <tt>http://.../foo.wt?hello=Hello&hello=World</tt> will result in
   * both values <tt>"Hello"</tt> and <tt>"World"</tt> to be
   * associated with the argument <tt>"hello"</tt>.
   *
   * \if cpp
   * \note The parameter map will be updated when the page is refreshed
   * (e.g. in non-Ajax sessions and when reload-is-new-session is false),
   * so you will have to make a copy of these values to preserve it across
   * different requests.
   * \endif
   *
   * \sa getParameterMap()
   */
  const Http::ParameterValues& getParameterValues(const std::string& name)
    const;

  /*! \brief Returns a single value for a query parameter.
   *
   * Returns the first value for a parameter, or \c 0 if the parameter is
   * not found.
   *
   * \if cpp
   * \note The parameter map will be updated when the page is refreshed
   * (e.g. in non-Ajax sessions and when reload-is-new-session is false),
   * so you will have to make a copy of the returned string if you want to
   * preserve this parameter across different requests.
   * \endif
   *
   * \sa getParameterValues()
   */
  const std::string *getParameter(const std::string& name) const;

  /*! \brief Returns the cookies from the environment.
   *
   * This returns all cookies that were present in initial request for
   * the application. Cookies set with WApplication::setCookie() are
   * not taken into consideration.
   *
   * Cookies allow you to persist information across sessions, but
   * note that not all clients may support cookies or may some clients
   * may be configured to block cookies.
   *
   * \sa supportsCookies(), getCookie(), getCookieValue()
   */
  const CookieMap& cookies() const { return cookies_; }

  /*! \brief Returns a cookie value.
   *
   * Returns 0 if no value was set for the given cookie.
   *
   * \sa getCookie()
   */
  const std::string *getCookie(const std::string& cookieName) const;

  /*! \brief Returns a header value.
   *
   * Returns a header value, or an empty string if the header was 
   * present.
   */
  const std::string headerValue(const std::string& field) const;

  /*! \brief Returns whether the browser has enabled support for cookies.
   *
   * When the user disables cookies during the visit of the page, this
   * value is not updated.
   *
   * \sa cookies(), getCookie()
   */
  bool supportsCookies() const { return doesCookies_; }

  /*! \brief Returns whether the browser has enabled support for JavaScript.
   *
   * This is the same as ajax(): %Wt only considers using JavaScript
   * when it has detected AJAX support.
   *
   * \sa ajax()
   */
  bool javaScript() const { return doesAjax_; }

  /*! \brief Returns whether the browser has enabled support for AJAX.
   *
   * Without support for JavaScript/AJAX, %Wt will still be able to
   * serve the application, but with one considerable limitation: only
   * the WTimer::timeout(), WInteractWidget::clicked(),
   * WApplication::internalPathChanged(), and WAbstractArea::clicked()
   * signals (and any derived signals) will generate events.
   *
   * Every event will cause the complete page to be rerendered.
   *
   * \sa javaScript()
   */
  bool ajax() const { return doesAjax_; }

  /*! \brief Returns whether the browser has support for WebGL.
   *
   * Support for WebGL is required for client-side rendering of
   * WGLWidget.
   */
  bool webGL() const { return webGLsupported_; }

  /*! \brief Returns the horizontal resolution of the client's screen.
   *
   * Returns -1 if screen width is not known.
   *
   * \sa screenHeight()
   */
  int screenWidth() const { return screenWidth_; }

  /*! \brief Returns the vertical resolution of the client's screen.
   *
   * Returns -1 if screen height is not known.
   *
   * \sa screenWidth()
   */
  int screenHeight() const { return screenHeight_; }

  /*! \brief Returns the browser-side DPI scaling factor
   *
   * Internet Explorer scales all graphics, fonts and other elements
   * on high-density screens to make them readable. This is controlled
   * by the DPI setting of the display. If all goes well, you do not
   * have to worry about this scaling factor. Unfortunately, not
   * all elements are scaled appropriately. The scaling factor is
   * supposed to be used only internally in %Wt and is in this interface
   * for informational purposes.
   *
   * \sa WVmlImage
   */
  double dpiScale() const { return dpiScale_; }

  /*! \brief Returns the preferred language indicated in the request
   *         header.
   *
   * The language is parsed from the HTTP <tt>Accept-Language</tt>
   * field, if present. If not, the locale is empty.
   *
   * If multiple languages are present, the one with the highest
   * "q"uality is assumed, and if a tie is present, the first one
   * is taken.
   *
   * \sa WApplication::setLocale()
   */
  const WLocale& locale() const { return locale_; }

  /*! \brief Returns the time zone offset as reported by the client.
   *
   * This returns the time offset that the client has relative to
   * UTC. A positive value thus means that the local time is ahead of
   * UTC.
   *
   * This requires JavaScript support.
   *
   * \sa WLocalDateTime::timeZoneOffset()
   */
  std::chrono::minutes timeZoneOffset() const { return timeZoneOffset_; }

  /*! \brief Returns the time zone name as reported by the client.
   *
   * \note This requires JavaScript support and is only supported by
   * browsers that implement the JavaScript Internationalization API.
   * No version of Internet Explorer supports this, but modern browsers
   * do. If not supported, this will return the empty string.
   */
  std::string timeZoneName() const { return timeZoneName_; }

  /*! \brief Returns the server host name that is used by the client.
   *
   * The hostname is the unresolved host name with optional port number,
   * which the browser used to connect to the application.
   *
   * Examples:
   *   - <tt>www.mydomain.com</tt>
   *   - <tt>localhost:8080</tt>
   *
   * For HTTP 1.1 requests, this information is fetched from the HTTP
   * <tt>Host</tt> header. If %Wt is configured behind a reverse
   * proxy, then the last entry in the HTTP <tt>X-Forwarded-Host</tt>
   * header field is used instead (to infer the name of the reverse
   * proxy instead).
   *
   * For HTTP 1.0 requests, the HTTP <tt>Host</tt> header is not
   * required. When not present, the server host name is inferred from
   * the configured server name, which defaults to the DNS name.
   */
  const std::string& hostName() const { return host_; }

  /*! \brief Returns the URL scheme used for the current request
   * (<tt>"http"</tt> or <tt>"https"</tt>).
   */
  const std::string& urlScheme() const { return urlScheme_; }

  /*! \brief Returns the user agent.
   *
   * The user agent, as reported in the HTTP <tt>User-Agent</tt> field.
   *
   * \sa agent()
   */
  const std::string& userAgent() const { return userAgent_; }

  /*! \brief Returns the referer.
   *
   * The referer, as reported in the HTTP <tt>Referer</tt> field.
   */
  const std::string& referer() const { return referer_; }

  /*! \brief Returns the accept header.
   *
   * The accept header, as reported in the HTTP <tt>Accept</tt> field.
   */
  const std::string& accept() const { return accept_; }

  /*! \brief Returns if the user agent is a (known) indexing spider bot.
   *
   * Note: currently the list of know bots is quite small. This method
   * is used internally to render the web application for optimal
   * indexing by bots:
   * - there is no detection for JavaScript, instead the application is
   *   directly served assuming no JavaScript support.
   * - session information is omitted from the Urls.
   * - no sessions are created (they are immediately stopped after the request
   *   has been handled).
   * - auto-generated <tt>id</tt> and <tt>name</tt> attributes are omitted
   *   from DOM nodes. In this way, the generated page is always exactly the
   *   same.
   */
  bool agentIsSpiderBot() const { return agent_ == UserAgent::BotAgent; }

  /*! \brief Returns the web server signature.
   *
   * The value of the CGI variable <tt>SERVER_SIGNATURE</tt>.
   *
   * Example: <tt>&lt;address&gt;Apache Server at localhost Port 80&lt;/address&gt;</tt>.
   */
  const std::string& serverSignature() const { return serverSignature_; }

  /*! \brief Returns the web server software.
   *
   * The value of the CGI variable <tt>SERVER_SOFTWARE</tt>.
   *
   * Example: <tt>"Apache"</tt>
   */
  const std::string& serverSoftware() const { return serverSoftware_; }

  /*! \brief Returns the email address of the server admin.
   *
   * The value of the CGI variable <tt>SERVER_ADMIN</tt>.
   *
   * Example: <tt>"root@localhost"</tt>
   */
  const std::string& serverAdmin() const { return serverAdmin_; }

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
  const std::string& clientAddress() const { return clientAddress_; }

  /*! \brief Returns the initial internal path.
   *
   * This is the internal path with which the application was started.
   *
   * For an application deployed at <tt>"/stuff/app.wt"</tt>, the following
   * two URLs are considered equivalent, and indicate an internal path 
   * <tt>"/this/there"</tt>:
   * \code
   * http://www.mydomain.com/stuff/app.wt/this/there
   * http://www.mydomain.com/stuff/app.wt/this/there
   * \endcode
   *
   * \if cpp
   * For the built-in httpd, when the application is deployed at a folder
   * (ending with '/'), only an exact matching path is routed to
   * the application (this can be changed since Wt 3.1.9, see
   * \ref wthttpd), making clean URLs impossible. Then, also the
   * following URL is supported (assuming deployment at <tt>"/stuff/"</tt>:
   * \code
   * http://www.mydomain.com/stuff/?_=/this/there
   * \endcode
   * \endif
   *
   * \sa WApplication::setInternalPath(), deploymentPath()
   */
  const std::string& internalPath() const { return internalPath_; }

  /*! \brief Returns the deployment path.
   *
   * This is the path at which the application is deployed.
   *
   * \sa internalPath().
   */
  const std::string& deploymentPath() const;

  /*! \brief Returns the version of the %Wt library.
   *
   * Example: <tt>"1.99.2"</tt>
   */
  static std::string libraryVersion();

  /*! \brief Returns the version of the %Wt library, broken down.
   *
   * The version of the %Wt library, broken down in its three numbers,
   *
   * Example: <i>series</i> = 1, <i>major</i> = 99, \p minor = 2.
   */
  void libraryVersion(int& series, int& major, int& minor) const;

  /*! \brief Returns a raw CGI environment variable.
   *
   * Retrieves the value for the given CGI environment variable (like
   * <tt>"SSL_CLIENT_S_DN_CN"</tt>), if it is defined, otherwise an
   * empty string.
   *
   * \sa serverSignature(), serverSoftware(), serverAdmin(),
   */
  std::string getCgiValue(const std::string& varName) const;

  /*! \brief The type of the content provided to the browser.
   *
   * This is here for backwards compatibility, but the implementation now
   * alwasy returns HTML5.
   */
  HtmlContentType contentType() const { return HtmlContentType::HTML5; }

  /*! \brief Returns the user agent type.
   *
   * This returns an interpretation of the userAgent(). It should be
   * used only for user-agent specific work-arounds (as a last
   * resort).
   *
   * \sa agentIsIE(), agentIsOpera(), agentIsGecko(), agentIsChrome(),
   *     agentIsSafari(), agentIsWebKit()
   */
  UserAgent agent() const { return agent_; }

  /*! \brief Returns whether the user agent is Microsoft Internet Explorer.
   *
   * \sa agent()
   */
  bool agentIsIE() const {
    return static_cast<unsigned int>(agent_) >=
      static_cast<unsigned int>(UserAgent::IEMobile) &&
      static_cast<unsigned int>(agent_) <
	static_cast<unsigned int>(UserAgent::Opera);
  }

  /*! \brief Returns whether the user agent is an older version of IE
   *
   * Returns whether the agent is an IE version older than the given version.

   * \sa agentIsIE()
   */
  bool agentIsIElt(int version) const {
    if (agentIsIE())
      return static_cast<unsigned int>(agent_) <
	static_cast<unsigned int>(UserAgent::IEMobile) + (version - 5);
    else
      return false;
  }

  /*! \brief Returns whether the user agent is Internet Explorer Mobile.
   *
   * Returns also \c true when the agent is Internet Explorer 5 or older.
   *
   * \sa agent()
   */
  bool agentIsIEMobile() const {
    return agent_ == UserAgent::IEMobile;
  }

  /*! \brief Returns whether the user agent is Opera.
   *
   * \sa agent()
   */
  bool agentIsOpera() const {
    return static_cast<unsigned int>(agent_) >=
      static_cast<unsigned int>(UserAgent::Opera) &&
      static_cast<unsigned int>(agent_) <
	static_cast<unsigned int>(UserAgent::Safari);
  }

  /*! \brief Returns whether the user agent is WebKit-based.
   *
   * Webkit-based browsers include Safari, Chrome, Arora and Konquerer
   * browsers.
   *
   * \sa agent()
   */
  bool agentIsWebKit() const {
    return static_cast<unsigned int>(agent_) >= 
      static_cast<unsigned int>(UserAgent::WebKit) && 
      static_cast<unsigned int>(agent_) < 
	static_cast<unsigned int>(UserAgent::Konqueror);
  }

   /*! \brief Returns whether the user agent is Mobile WebKit-based.
   *
   * Mobile Webkit-based browsers include the Android Mobile WebKit
   * and the iPhone Mobile WebKit browsers.
   *
   * \sa agent()
   */
  bool agentIsMobileWebKit() const {
    return static_cast<unsigned int>(agent_) >= 
      static_cast<unsigned int>(UserAgent::MobileWebKit) &&  
      static_cast<unsigned int>(agent_) < 
	static_cast<unsigned int>(UserAgent::Konqueror);
  }

  /*! \brief Returns whether the user agent is Safari.
   *
   * \sa agent()
   */
  bool agentIsSafari() const {
    return static_cast<unsigned int>(agent_) >= 
      static_cast<unsigned int>(UserAgent::Safari) &&  
      static_cast<unsigned int>(agent_) < 
	static_cast<unsigned int>(UserAgent::Chrome0);
  }

  /*! \brief Returns whether the user agent is Chrome.
   *
   * \sa agent()
   */
  bool agentIsChrome() const {
    return static_cast<unsigned int>(agent_) >= 
      static_cast<unsigned int>(UserAgent::Chrome0) &&  
      static_cast<unsigned int>(agent_) < 
	static_cast<unsigned int>(UserAgent::Konqueror);
  }
  
  /*! \brief Returns whether the user agent is Gecko-based.
   *
   * Gecko-based browsers include Firefox.
   *
   * \sa agent()
   */
  bool agentIsGecko() const {
    return static_cast<unsigned int>(agent_) >= 
      static_cast<unsigned int>(UserAgent::Gecko) &&  
      static_cast<unsigned int>(agent_) < 
	static_cast<unsigned int>(UserAgent::BotAgent);
  }

#ifndef WT_TARGET_JAVA
  /*! \brief Returns the server.
   *
   * This returns the server environment of this session.
   */
#else
  /*! \brief Returns the servlet.
   *
   * This returns the servlet environment of this session.
   */
#endif // WT_TARGET_JAVA
  WServer *server() const;

#ifndef WT_TARGET_JAVA
  /*! \brief Returns information on the SSL client certificate or \c nullptr
   *  if no authentication took place.
   *
   * This function will return \c nullptr if no verification took place, %Wt
   * was compiled without SSL support, or the web server was
   * configured without client SSL certificates.
   *
   * This method may return a pointer to a WSslInfo object, while the
   * authentication may have failed. This depends on the configuration
   * of the web server. It is therefore important to always check the
   * verification result with WSslInfo::clientVerificationResult().
   *
   * Remember that WEnvironment is 'const', thus the object returned
   * by this function will represent the SSL information of the
   * first request for this session. It will not be updated during
   * the lifetime of the session.
   *
   * The object returned is owned by WEnvironment and will be deleted
   * when WEnvironment is destroyed (= at the end of the session).
   *
   * \includedoc ssl_client_headers.dox
   *
   * \sa Wt::Http::Request::sslInfo()
   */
  WSslInfo *sslInfo() const {
    return sslInfo_.get();
  }
#endif

  /*! \brief Returns whether internal paths are implemented using URI fragments.
   *
   * This may be the case for older non-HTML5 browsers which do not support
   * HTML5 History APIs.
   */
  bool internalPathUsingFragments() const { 
    return internalPathUsingFragments_; 
  }

  /*! \brief Returns whether this agent supports CSS3 animations.
   */
  bool supportsCss3Animations() const;

  virtual Signal<WDialog *>& dialogExecuted() const;
  virtual Signal<WPopupMenu *>& popupExecuted() const;

  /*! \brief Returns whether this is a mocked test environment.
   */
  virtual bool isTest() const;

protected:
  WebSession *session_;
  bool        doesAjax_;
  bool        doesCookies_;
  bool        internalPathUsingFragments_;
  UserAgent   agent_;
  int         screenWidth_;
  int         screenHeight_;
  double      dpiScale_;
  std::string queryString_;
  bool        webGLsupported_;

  Http::ParameterMap parameters_;
  CookieMap   cookies_;

  WLocale locale_;
  std::chrono::minutes timeZoneOffset_;
  std::string timeZoneName_;
  std::string host_;
  std::string userAgent_;
  std::string urlScheme_;
  std::string referer_;
  std::string accept_;
  std::string serverSignature_;
  std::string serverSoftware_;
  std::string serverAdmin_;
  std::string clientAddress_;
  std::string pathInfo_;
  std::string internalPath_;
  std::string publicDeploymentPath_;

#ifndef WT_TARGET_JAVA
  std::unique_ptr<WSslInfo> sslInfo_;
#endif

  WEnvironment();
  virtual ~WEnvironment();

  void setUserAgent(const std::string& agent);
  void setInternalPath(const std::string& path);
 
private:
  WEnvironment(WebSession *session);

  void init(const WebRequest& request);
  void updateHostName(const WebRequest& request);
  void updateUrlScheme(const WebRequest& request);
  void enableAjax(const WebRequest& request);

  bool agentSupportsAjax() const;
  static void parseCookies(const std::string& cookie,
			   std::map<std::string, std::string>& result);

  friend class WebController;
  friend class WebRenderer;
  friend class WebSession;
  friend class WApplication;
  friend class Http::Request;
};

}

#endif // WENVIRONMENT_H_
