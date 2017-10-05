// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSERVER_H_
#define WSERVER_H_

#include <Wt/WApplication.h>
#include <Wt/WException.h>
#include <Wt/WLogger.h>

#include <chrono>

namespace http {
  namespace server {
    class Server;
  }
}
namespace Wt {

class Configuration;
class WebController;
class WIOService;
/*! \class WServer Wt/WServer.h Wt/WServer.h
 *  \brief A class encapsulating a web application server.
 *
 * This server class represents an instance of an application server.
 *
 * It offers support for multiple application entry points and control
 * over starting and stopping the server. This may be used as an
 * alternative to using WRun() when you wish to support multiple
 * application entry points, or for integrating a %Wt (stand-alone
 * httpd) server application into an existing application, with control
 * over starting and stopping the server as appropriate.
 *
 * As an example usage, consider the implementation of WRun(), which
 * starts the server until a Ctrl-C is pressed or a termination signal
 * has been received, or a restart is indicated using SIGHUP or a changed
 * binary (argv[0]):
 *
 * \code
int WRun(int argc, char *argv[], ApplicationCreator createApplication)
{
  try {
    // use argv[0] as the application name to match a suitable entry
    // in the Wt configuration file, and use the default configuration
    // file (which defaults to /etc/wt/wt_config.xml unless the environment
    // variable WT_CONFIG_XML is set)
    WServer server(argv[0]);

    // WTHTTP_CONFIGURATION is e.g. "/etc/wt/wthttpd"
    server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

    // add a single entry point, at the default location (as determined
    // by the server configuration's deploy-path)
    server.addEntryPoint(Wt::EntryPointType::Application, createApplication);
    if (server.start()) {
      int sig = WServer::waitForShutdown(argv[0]);

      std::cerr << "Shutdown (signal = " << sig << ")" << std::endl;
      server.stop();

      if (sig == SIGHUP)
	WServer::restart(argc, argv, environ);
    }
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }
}
 * \endcode
 */
class WServer
{
public:
  typedef std::function<std::string (std::size_t max_length, int purpose)>
    SslPasswordCallback;

  /*! \class Exception
   *  \brief Server %Exception class.
   */
  class WT_API Exception : public WException
  {
  public:
    Exception(const std::string& what);
  };

#ifndef WT_TARGET_JAVA
  /*! \class SessionInfo
   *
   * \brief Contains the information for one session.
   */
  struct WT_API SessionInfo
  {
    int64_t     processId; //!< The process id of the process the session is running in.
    std::string sessionId; //!< The session id.
  };
#endif // WT_TARGET_JAVA

  /*! \brief Creates a new server instance.
   *
   * The \p wtApplicationPath is used to match specific
   * application-settings in the %Wt configuration file. If no
   * specific match could be found, the general settings are used
   * (corresponding to the '*' selector).
   *
   * The %Wt application configuration is read from the
   * \p wtConfigurationFile. If empty, this defaults to the value
   * configured at build time.
   *
   * For more information on configuring %Wt applications, see \ref
   * configuration_sec "Configuration".
   *
   * \throws Exception : indicates a configuration problem.
   *
   * \sa setServerConfiguration()
   */
  WTCONNECTOR_API
    WServer(const std::string& wtApplicationPath = std::string(),
	    const std::string& wtConfigurationFile = std::string());

  WServer(const WServer &) = delete;

  /*! \brief Creates a new server instance and configures it.
   *
   * This is equivalent to:
   * \code
    WServer server(argv[0]);
    server.setServerConfiguration(argc, argv, wtConfigurationFile);
    \endcode
   *
   * \throws Exception : indicates a configuration problem.
   *
   * \sa WServer(const std::string&, const std::string&)
   * \sa setServerConfiguration()
   */
  WTCONNECTOR_API
    WServer(int argc, char *argv[], const std::string& wtConfigurationFile = std::string());

  /*! \brief Destructor.
   *
   * If the server was still running, it is stopped first by calling
   * stop(). It is probably safer to call stop() first yourself, since
   * this allows exceptions to be caught.
   *
   * \sa isRunning(), stop()
   */
  WTCONNECTOR_API virtual ~WServer();

#ifndef WT_TARGET_JAVA
  /*! \brief Sets the I/O service.
   *
   * The server will use an I/O service for scheduling functions into a
   * thread-pool, and to implement asynchronous networking, whose call-back
   * funtions are scheduled in the same thread pool.
   *
   * By default, a server will create its own I/O service, but you may
   * configure it to reuse another I/O service.
   */
  WT_API void setIOService(WIOService& ioService);

  /*! \brief Returns the I/O service.
   *
   * \sa setIOService()
   */
  WT_API WIOService& ioService();
#endif

  /*! \brief Returns the server instance.
   *
   * Returns the single server instance. This may be useful when using
   * WRun(), which does not provide direct access to the instantiated
   * server, but still you want to use functions like
   * post().
   *
   * \note When instantiating multiple servers, this will simply return the
   *       last instance. You probably want to avoid this function then.
   */
  static WServer *instance() { return instance_; }

#ifndef WT_TARGET_JAVA
  /*! \brief Configures the HTTP(S) server or FastCGI process.
   *
   * Configures the HTTP(S) server using command-line arguments, a
   * configuration file, or both. The valid options are described in
   * \ref config_wthttpd "Built-in httpd configuration".
   *
   * The applications themselves are configured using the configuration
   * file passed to the constructor.
   *
   * The server configuration must be set before any other
   * functionality can be used.
   *
   * In case of FastCGI deployment, the \p serverConfigurationFile
   * argument is ignored, and depending on the command-line arguments,
   * this process may become a FastCGI protocol relay process which
   * never returning from this call but directs the FastCGI stream to
   * the correct session, rather than a Wt application server.
   *
   * \throws Exception : indicates a configuration problem.
   */
  WTCONNECTOR_API
    void setServerConfiguration(int argc, char *argv[],
				const std::string& serverConfigurationFile
				= std::string());

  /*! \brief Binds an entry-point to a callback function to create
   *         a new application.
   *
   * The \p path is the local URL at which the application is
   * deployed: when a user visits this URL, the callback will be
   * called to create a new application. If empty, the URL is inferred
   * from the server configuration's deploy-path (see also \ref
   * config_wthttpd "Built-in httpd configuration").
   *
   * The path must start with a '/'.
   *
   * The optional \p favicon is a URL path (which should not
   * contain the host part!) to a favicon, which is the icon displayed
   * in the browser for your application. Alternatively, you may
   * specify a favicon using the "favicon" property in the
   * configuration file (see als \ref config_general "EntryPointType::Application
   * settings (wt_config.xml)").
   *
   * \sa removeEntryPoint()
   */
  WT_API void addEntryPoint(EntryPointType type, ApplicationCreator callback,
		            const std::string& path = std::string(),
                            const std::string& favicon = std::string());
#endif

  /*! \brief Binds a resource to a fixed path.
   *
   * Resources may either be private to a single session or
   * public. Use this method to add a public resource with a fixed
   * path.
   *
   * \sa removeEntryPoint()
   */
  WT_API void addResource(WResource *resource, const std::string& path);

  /*! \brief Removes an entry point.
   *
   * Use this method to remove an entry point or static resource.
   *
   * In a multi-threaded environment, this may only be done when the
   * server is not running.
   *
   * When a resource has been bound to the path, the resource will not
   * be deleted.
   *
   * \sa addEntryPoint(), addResource()
   */
  WT_API void removeEntryPoint(const std::string& path);

#ifndef WT_TARGET_JAVA
  /*! \brief Starts the server in the background.
   *
   * Returns whether the server could be successfully started.
   *
   * \throws Exception : indicates a problem starting the server.
   *
   * \sa isRunning(), stop()
   */
  WTCONNECTOR_API bool start();

  /*! \brief Stops the server.
   *
   * All active application sessions are terminated cleanly, and the
   * HTTP(S) server is shut down.
   *
   * \note This will also stop the underlying ioService(), and will block
   *       until all pending tasks have completed.
   *
   * \throw Exception : indicates a problem while stopping the server.
   *
   * \sa isRunning(), start()
   */
  WTCONNECTOR_API void stop();

  /*! \brief Returns whether the server is running.
   *
   * \sa start(), stop()
   */
  WTCONNECTOR_API bool isRunning() const;

  /*! \brief Starts the server, waits for shutdown, then stops the server.
   *
   * This is equivalent to:
   * \code
    if (start()) {
      WServer::waitForShutdown();
      stop();
    }
    \endcode
   * \sa start()
   * \sa stop()
   * \sa waitForShutdown()
   */
  WTCONNECTOR_API void run();

  /*! \brief Resumes the server.
   *
   * This closes and reopens the listen socket(s) for accepting new
   * TCP and/or SSL connections. This may be needed when the OS (like
   * IPhoneOS) has closed the sockets while suspending the
   * application.
   */
  WTCONNECTOR_API void resume();

  /*! \brief Waits for a shutdown signal.
   *
   * This static method blocks the current thread, waiting for a
   * shutdown signal. The implementation and details are platform
   * dependent, but this is usually Ctrl-C (SIGINT) or SIGKILL.
   *
   * This method is convenient if you want to customize how the server
   * is started (by instantiating a WServer object yourself, instead
   * of using Wt::WRun()), but still want to use %Wt as a standalone
   * server that cleanly terminates on interruption.
   *
   * This will also catch SIGHUP, to reread the configuration file.
   */
  WT_API static int waitForShutdown();

  /*! \brief A utility method to restart.
   *
   * This will result the application with the new image (argv[0]), effectively
   * loading a newly deployed version. <i>(Experimental, UNIX only)</i>
   */
  WT_API static void restart(int argc, char **argv, char **envp);

  /*! \brief Returns the server HTTP port number.
   *
   * Returns -1 if the port is not known (i.e. because the connector is
   * not aware of how the http server is configured).
   *
   * \note If the server listens on multiple ports, only the first
   *       port is returned.
   */
  WTCONNECTOR_API int httpPort() const;

  WT_API void setAppRoot(const std::string& path);

  /*! \brief Returns the approot special property
   *
   * \sa WApplication::appRoot()
   */
  WT_API std::string appRoot() const;

  /*! \brief Posts a function to a session.
   *
   * This is a thread-safe method to post a particular event
   * (implemented as a function object) to be run within the context
   * of a session, identified by its WApplication::sessionId(). The
   * method will safely handle the case where the session is being
   * terminated, and the session lock will be taken to execute the
   * function in the context of the session (with
   * WApplication::instance() pointing to the correct application),
   * just as with a request initiated by the browser. You will
   * typically also want to push the changes to the client using
   * server-initiated updates (WApplication::triggerUpdate()).
   *
   * The method returns immediately, and the function will be run
   * within the thread-pool that handles incoming web requests. In
   * this way, it avoids dead-lock scenarios.
   *
   * If a \p fallbackFunction is specified then in case the session
   * is dead, it is called instead.
   * 
   * This provides a good alternative to grabbing the update lock of
   * an application to directly push changes to a session out of its
   * event loop.
   *
   * Note that if you post an event to a method of a widget (or other
   * object), it may still be that the targeted object has been
   * deleted, if the life-time of that object is not the same as the
   * life-time of the application. It may be useful to protect
   * yourself from this by using WApplication::bind().
   */
  WT_API void post(const std::string& sessionId,
	           const std::function<void ()>& function,
	           const std::function<void ()>& fallBackFunction
	             = std::function<void ()>());

  /*! \brief Posts a function to all currently active sessions.
   *
   * \sa post()
   */
  WT_API void postAll(const std::function<void ()>& function);

  WT_API void schedule(std::chrono::steady_clock::duration millis,
		       const std::string& sessionId,
		       const std::function<void ()>& function,
		       const std::function<void ()>& fallBackFunction
		         = std::function<void ()>());

  /*! \brief Change input method for server certificate passwords (http backend)
   *
   * The private server identity key may be protected by a password. If you
   * want to control how the password is retrieved, set a password handler
   * by calling this function. If no password handler is set, the OpenSSL
   * default handler will be used, which asks to enter the password on stdio.
   *
   * This function must be called before calling start().
   *
   * The max_length parameter is informational and indicates that the
   * underlying implementation will truncate the password to this length.
   */
  WTCONNECTOR_API void setSslPasswordCallback(const SslPasswordCallback& cb);

#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
  /*! \brief Reads a configuration property.
   *
   * As properties are unique to an executable location, they are defined
   * from the moment that setServerConfiguration() is invoked. Use this
   * method to access configuration properties outside of an active
   * session, e.g. from within the main() function.
   *
   * \sa WApplication::readConfigurationProperty()

   */
  WT_API bool readConfigurationProperty(const std::string& name,
					std::string& value) const;

#else
  /*! \brief Reads a configuration property.
   *
   * Tries to read a configured value for the property \p name. If no
   * value was configured, the default \p value is returned.
   *
   * \sa WApplication::readConfigurationProperty()
   */
  std::string *readConfigurationProperty(const std::string& name,
					 const std::string& value);
#endif // WT_TARGET_JAVA

  /*! \brief Sets the resource object that provides localized strings.
   *
   * This is used only for WString::tr() used from within static
   * resources.
   *
   * The default value is 0.
   */
  WT_API void setLocalizedStrings(const std::shared_ptr<WLocalizedStrings>&
				  stringResolver);
  
  /*! \brief Sets the resource object that provides localized strings.
   *
   * \sa setLocalizedStrings()
   */
  WT_API std::shared_ptr<WLocalizedStrings> localizedStrings() const;

#ifndef WT_TARGET_JAVA

  /*! \brief Retrieve information on all sessions.
   *
   * This is only implemented for the wthttp connector.
   *
   * If the dedicated process session policy is used, only the original
   * process has access to the full list of sessions. Public resources
   * (those registered with addResource()) run in the original process,
   * so they can access this list.
   */
  WTCONNECTOR_API std::vector<SessionInfo> sessions() const;

  /*! \brief Returns the logger instance.
   *
   * This is the logger class used in WApplication::log() and
   * Wt::log() functions.
   */
  WT_API WLogger& logger();

  /*! \brief Adds an entry to the log.
   *
   * \sa Wt::log(), WApplication::log()
   */
  WT_API WLogEntry log(const std::string& type) const;

  WT_API void initLogger(const std::string& logFile,
			 const std::string& logConfig);

  /*!
   * \brief Reflects whether the current process is a dedicated session process
   */
  WT_API bool dedicatedSessionProcess() const;
#endif // WT_TARGET_JAVA

  WT_API bool expireSessions();

  WT_API Configuration& configuration();

  WT_API WebController *controller();

#ifndef WT_TARGET_JAVA
  WT_API void scheduleStop();
#endif // WT_TARGET_JAVA

#ifdef WT_WIN32
  WT_API static void terminate();
#endif // WT_WIN32

#ifdef WT_TARGET_JAVA
  std::string getContextPath();
  int servletMajorVersion();
#endif

private:
  WebController *webController_;

#ifndef WT_TARGET_JAVA
  WLogger logger_;
#endif // WT_TARGET_JAVA

  std::string application_, configurationFile_, appRoot_, description_;
  Configuration *configuration_;
  std::shared_ptr<WLocalizedStrings> localizedStrings_;

  bool ownsIOService_;
  WIOService *ioService_;

  bool dedicatedProcessEnabled_;

  struct Impl;
  Impl *impl_;

  WT_API void setConfiguration(const std::string& file,
			       const std::string& application);

  WT_API void setConfiguration(const std::string& file);
  const std::string& configurationFile() const {
    return configurationFile_;
  }

  WT_API void init(const std::string& wtApplicationPath,
	           const std::string& configurationFile);
  WT_API void destroy();
  WT_API void setCatchSignals(bool catchSignals);

  WT_API static WServer *instance_;
  std::function<std::string (std::size_t max_length, int purpose)> sslPasswordCallback_;
#ifndef WT_TARGET_JAVA
  std::function<void ()> stopCallback_;
#endif // WT_TARGET_JAVA
};

}
#endif // WSERVER_H_ 
