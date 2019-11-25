// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTEST_ENVIRONMENT_H_
#define WTEST_ENVIRONMENT_H_

#include <string>
#include <map>
#include <vector>

#include <Wt/WEnvironment.h>
#include <Wt/WServer.h>

namespace Wt {

class WebController;

namespace Test {

/*! \class WTestEnvironment Wt/Test/WTestEnvironment.h Wt/Test/WTestEnvironment.h
 *  \brief An environment for testing purposes.
 *
 * This environment is useful for use in automated (integration/unit)
 * tests: you may configure its properties and pass it to the
 * constructor of an application.
 *
 * This is useful for automated test-cases:
 * \if cpp
 * \code
 * void testX() {
 *   Test::WTestEnvironment environment;
 *   MyApplication app(environment);
 *   ...
 * }
 * \endcode
 * \elseif java
 * \code
 * void testX() {
 *   WTestEnvironment environment(new Configuration());
 *   MyApplication app(environment);
 *   ...
 * }
 * \endcode
 * \endif
 *
 * \sa WEnvironment, WApplication::WApplication(const WEnvironment&)
 */
class WTCONNECTOR_API WTestEnvironment : public WEnvironment
{
public:
#ifndef WT_TARGET_JAVA
  /*! \brief Default constructor.
   *
   * Constructs a test environment that resembles FireFox 3.0 with default
   * settings.
   *
   * After construction, but before passing it to the constructor of a
   * WApplication, you can change any of the environment properties using
   * the setter methods.
   */
  WTestEnvironment(EntryPointType type = EntryPointType::Application);

  /*! \brief Constructor with custom configuration
   *
   * Constructs a test environment that resembles FireFox 3.0 with default
   * settings.
   *
   * The <i>applicationPath</i> is the simulated application path used to
   * match <tt><application-settings></tt> in the configuration file. The
   * configuration file points to a wt_config.xml configuration file.
   * The <i>type</i> indicates the application type.
   *
   * After construction, but before passing it to the constructor of a
   * WApplication, you can change any of the environment properties using
   * the setter methods.
   */
  WTestEnvironment(const std::string& applicationPath,
		   const std::string& configurationFile,
		   EntryPointType type = EntryPointType::Application);

#else
  /*! \brief Default constructor.
   *
   * Constructs a test environment that resembles FireFox 3.0 with default
   * settings.
   *
   * After construction, but before passing it to the constructor of a
   * WApplication, you can change any of the environment properties using
   * the setter methods.
   */
  WTestEnvironment(Configuration *configuration,
		   EntryPointType type = EntryPointType::Application);

  /*! \brief Closes the test environment.
   *
   * Destroys the test environment. This will allow the environment and the
   * application under test to be garbage collected.
   */
  void close();
#endif // WT_TARGET_JAVA

  /*! \brief Destructor.
   */
  ~WTestEnvironment();

  /*! \brief Sets parameters to the application.
   *
   * The default value is an empty map.
   *
   * \sa getParameterMap()
   */
  void setParameterMap(const Http::ParameterMap& parameters);

  /*! \brief Sets HTTP cookies.
   *
   * The default value is an empty map.
   *
   * \sa cookies()
   */
  void setCookies(const CookieMap& cookies);

  /*! \brief Sets a HTTP header value.
   *
   * The default value is no headers.
   *
   * \sa headerValue()
   */
  void setHeaderValue(const std::string& value);

  /*! \brief Sets whether cookies are supported.
   *
   * The default value is <i>true</i>.
   *
   * \sa supportsCookies()
   */
  void setSupportsCookies(bool enabled);

  /*! \brief Sets whether AJAX is supported.
   *
   * The default value is <i>true</i>.
   *
   * \sa ajax()
   */
  void setAjax(bool enabled);

  /*! \brief Sets the display's DPI scale.
   *
   * The default value is 1.
   *
   * \sa dpiScale()
   */
  void setDpiScale(double dpiScale);

  /*! \brief Sets the locale.
   *
   * \if cpp
   * The default value is WLocale("en").
   * \endif
   *
   * \if java
   * The default value is the English locale ("en").
   * \endif
   *
   * \sa locale()
   */
  void setLocale(const WLocale& locale);

  /*! \brief Sets the host name.
   *
   * The default value is "localhost".
   *
   * \sa hostName()
   */
  void setHostName(const std::string& hostName);

  /*! \brief Sets the URL scheme.
   *
   * The default value is "http".
   *
   * \sa urlScheme()
   */
  void setUrlScheme(const std::string& scheme);

  /*! \brief Sets the user agent.
   *
   * The default value is no "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.0.11) Gecko/2009060309 Ubuntu/9.04 (jaunty) Firefox/3.0.11".
   *
   * \sa userAgent()
   */
  void setUserAgent(const std::string& userAgent);

  /*! \brief Sets the referer.
   *
   * The default value is "".
   *
   * \sa referer()
   */
  void setReferer(const std::string& referer);

  /*! \brief Sets the accept header.
   *
   * The default value is
   * "text/html,application/xhtml+xml,application/xml;q=0.9,*<span>/</span>*;q=0.8".
   *
   * \sa accept()
   */
  void setAccept(const std::string& accept);

  /*! \brief Sets the server signature.
   *
   * The default value is "None (WTestEnvironment)".
   *
   * \sa serverSignature()
   */
  void setServerSignature(const std::string& signature);

  /*! \brief Sets the server software.
   *
   * The default value is "None (WTestEnvironment)".
   *
   * \sa serverSoftware()
   */
  void setServerSoftware(const std::string& software);

  /*! \brief Sets the server admin.
   *
   * The default value is "your@onyourown.here".
   *
   * \sa serverAdmin()
   */
  void setServerAdmin(const std::string& serverAdmin);

  /*! \brief Sets the client address.
   *
   * The default value is "127.0.0.1".
   *
   * \sa clientAddress()
   */
  void setClientAddress(const std::string& clientAddress);

  /*! \brief Sets the initial internal path.
   *
   * The default value is "".
   *
   * \sa internalPath()
   */
  void setInternalPath(const std::string& internalPath);

#ifndef WT_TARGET_JAVA
  /*! \brief Sets the server's appRoot
   *
   * \sa WServer::appRoot()
   */
  void setAppRoot(const std::string& appRoot);

  /*! \brief Sets the docRoot
   *
   * \sa WApplication::docRoot()
   */
  void setDocRoot(const std::string& docRoot);
#endif // WT_TARGET_JAVA

  /*! \brief %Signal used to test a dialog/messagebox reentrant event
   *         loop.
   *
   * This signal is emitted when a dialog or message box is being
   * executed using WDialog::exec() or WMessageBox::exec(), and allows
   * you to interact with the dialog contents.
   *
   * In the end, the dialog should be closed while executing this
   * signal, (calling done() directly or indirectly) so that the main
   * event loop can continue.
   */
  virtual Signal<WDialog *>& dialogExecuted() const override;

  /*! \brief %Signal used to test a popup menu reentrant event loop.
   *
   * This signal is emitted when a popup menu is being executed using
   * WPopupMenu::exec(), and allows you to interact with the popup
   * menu (i.e. to select an option).
   *
   * \sa dialogExecuted()
   */
  virtual Signal<WPopupMenu *>& popupExecuted() const override;

  /*! \brief Simulates the end of a request by the main event loop.
   *
   * The environemnt (and application is) started from within the main
   * event loop. To simulate the delivery of events posted to the
   * application-under-test, by WServer::post(), you need to simulate
   * the release of the session lock.
   * 
   * \sa startRequest()
   */
  void endRequest();

  /*! \brief Simulates the start of a new request by the main event loop.
   *
   * \sa endRequest()
   */
  void startRequest();

  /*! \brief Simulates the presence of the session ID in the URL.
   *
   * A session ID in the URL should cause a trampoline to be used for
   * references to external servers.
   *
   * The default value is \c false.
   */
  void setSessionIdInUrl(bool sessionIdInUrl);

private:
  std::shared_ptr<WebSession> theSession_;

#ifndef WT_TARGET_JAVA
  WServer *server_;
#endif

  WebController *controller_;

  mutable Signal<WDialog *> dialogExecuted_;
  mutable Signal<WPopupMenu *> popupExecuted_;

  virtual bool isTest() const override;

  void init(EntryPointType type);
};

}

}

#endif // WTEST_ENVIRONMENT_H_
