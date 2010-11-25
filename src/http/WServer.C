// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WServer"
#include "Wt/WResource"

#include <iostream>
#include <string>

#if !defined(_WIN32)
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#endif // !_WIN32

#ifdef WT_THREADED
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#endif // WT_THREADED

#include <boost/bind.hpp>

#include "Connection.h"
#include "Server.h"
#include "Configuration.h"
#include "../web/Configuration.h"
#include "WebController.h"
#include "HTTPStream.h"

#ifdef WT_THREADED
typedef boost::thread thread_t;
#endif
namespace {
  bool gdb = false;

  static std::string getWtConfigXml(int argc, char *argv[],
				    const std::string& configurationFile)
  {
    std::string wt_config_xml;
    Wt::WLogger stderrLogger;
    stderrLogger.setStream(std::cerr);
    
    http::server::Configuration serverConfiguration(stderrLogger, true);
    serverConfiguration.setOptions(argc, argv, configurationFile);
    
    return serverConfiguration.configPath();
  }

}

namespace Wt {

struct WServerImpl {
  WServerImpl(const std::string& wtApplicationPath,
	      const std::string& wtConfigurationFile)
    : applicationPath_(wtApplicationPath),
      wtConfigurationFile_(wtConfigurationFile),
      wtConfiguration_(0),
      webController_(0),
      serverConfiguration_(0),
      server_(0)
  { }
  
  ~WServerImpl() {
    delete serverConfiguration_;
    delete webController_;
    delete wtConfiguration_;
  }

  void readConfiguration(const WServer *server) {
    readConfiguration(0, 0, std::string(), server);
  }

  void readConfiguration(int argc, char *argv[],
			 const std::string& serverConfigurationFile,
			 const WServer *server)
  {
    if (!wtConfiguration_) {
      if (wtConfigurationFile_.empty())
	wtConfigurationFile_ = getWtConfigXml(argc, argv,
					      serverConfigurationFile);

      wtConfiguration_
	= new Wt::Configuration(applicationPath_, "",
				wtConfigurationFile_,
				Wt::Configuration::WtHttpdServer,
				"Wt: initializing built-in httpd");

      webController_
	= new Wt::WebController(*wtConfiguration_,
				const_cast<WServer *>(server), &stream_);

      serverConfiguration_
	= new http::server::Configuration(wtConfiguration_->logger());

      if (argc != 0)
	serverConfiguration_->setOptions(argc, argv, serverConfigurationFile);
    }
  }

  std::string    applicationPath_, wtConfigurationFile_;
  Configuration *wtConfiguration_;
  HTTPStream     stream_;
  WebController *webController_;

  http::server::Configuration *serverConfiguration_;
  http::server::Server        *server_;
#ifdef WT_THREADED
  thread_t **threads_;
#endif
};

WServer::WServer(const std::string& applicationPath,
		 const std::string& wtConfigurationFile)
  : impl_(new WServerImpl(applicationPath, wtConfigurationFile))
{ }

WServer::~WServer()
{
  if (impl_->server_) {
    try {
      stop();
    } catch (...) {
      std::cerr << "WServer::~WServer: oops, stop() threw exception!"
		<< std::endl;
    }
  }

  delete impl_;
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string& serverConfigurationFile)
{
  impl_->readConfiguration(argc, argv, serverConfigurationFile, this);
}

void WServer::addEntryPoint(EntryPointType type, ApplicationCreator callback,
			    const std::string& path, const std::string& favicon)
{
  if (!path.empty() && !boost::starts_with(path, "/")) 
    throw WServer::Exception("WServer::addEntryPoint() error: "
			     "deployment path should start with \'/\'");

  impl_->readConfiguration(this);
  impl_->wtConfiguration_->addEntryPoint(EntryPoint(type, callback,
						    path, favicon));
}

void WServer::addResource(WResource *resource, const std::string& path)
{
  if (!boost::starts_with(path, "/")) 
    throw WServer::Exception("WServer::addResource() error: "
			     "static resource path should start with \'/\'");

  resource->setInternalPath(path);

  impl_->readConfiguration(this);
  impl_->wtConfiguration_->addEntryPoint(EntryPoint(resource, path));
}

bool WServer::start()
{
  impl_->readConfiguration(this);

  gdb = impl_->serverConfiguration_->gdb();

  if (isRunning()) {
    std::cerr << "WServer::start() error: server already started!" << std::endl;
    return false;
  }

#ifndef WIN32
  srand48(getpid());
#endif

  // Override sessionIdPrefix setting
  if (!impl_->serverConfiguration_->sessionIdPrefix().empty())
    impl_->wtConfiguration_->setSessionIdPrefix
      (impl_->serverConfiguration_->sessionIdPrefix());

  // Set default entry point
  impl_->wtConfiguration_->setDefaultEntryPoint
    (impl_->serverConfiguration_->deployPath());

  try {
    impl_->server_ = new http::server::Server(*impl_->serverConfiguration_,
					      *impl_->wtConfiguration_,
                                              *impl_->webController_);
#ifndef WT_THREADED
    impl_->serverConfiguration_->log("warn")
      << "No boost thread support, running in main thread.";

    impl_->server_->run();

    delete impl_->server_;
    impl_->server_ = 0;

    return false;
#else // WT_THREADED

#if !defined(_WIN32)
    // Block all signals for background threads.
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif // _WIN32

    int NUM_THREADS = impl_->serverConfiguration_->threads();

    impl_->threads_ = new thread_t *[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i)
      impl_->threads_[i] =
	new thread_t(boost::bind(&http::server::Server::run, impl_->server_));

#if !defined(_WIN32)
    // Restore previous signals.
    pthread_sigmask(SIG_SETMASK, &old_mask, 0);
#endif // _WIN32

#endif // WT_THREADED

    return true;
  } catch (asio_system_error& e) {
    throw Exception(std::string("Error (asio): ") + e.what());
  } catch (std::exception& e) {
    throw Exception(std::string("Error: ") + e.what());
  }
}

bool WServer::isRunning() const
{
  return impl_->server_;
}

#if defined(_WIN32) && defined(WT_THREADED)

boost::mutex     terminationMutex;
boost::condition ctrlCHit;
boost::condition serverStopped;

BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
  switch (ctrl_type)
  {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    {
      boost::mutex::scoped_lock terminationLock(terminationMutex);

      ctrlCHit.notify_all(); // should be just 1
      return TRUE;
    }
  default:
    return FALSE;
  }
}
#endif

void WServer::stop()
{
  if (!isRunning()) {
    std::cerr << "WServer::stop() error: server not yet started!" << std::endl;
    return;
  }

#ifdef WT_THREADED
  try {
    // Stop the Wt application server (cleaning up all sessions).
    impl_->webController_->forceShutdown();

    // Stop the server.
    impl_->server_->stop();

    int NUM_THREADS = impl_->serverConfiguration_->threads();
    for (int i = 0; i < NUM_THREADS; ++i) {
      impl_->threads_[i]->join();
      delete impl_->threads_[i];
    }

    delete[] impl_->threads_;
    impl_->threads_ = 0;

    delete impl_->server_;
    impl_->server_ = 0;
  } catch (asio_system_error& e) {
    throw Exception(std::string("Error (asio): ") + e.what());
  } catch (std::exception& e) {
    throw Exception(std::string("Error: ") + e.what());
  }

#if defined(_WIN32)
  serverStopped.notify_all();
#endif // WIN32

#else // WT_THREADED
  impl_->webController_->forceShutdown();
  impl_->server_->stop();
#endif // WT_THREADED
}

int WServer::httpPort() const
{
  return impl_->server_->httpPort();
}

void WServer::restart(int argc, char **argv, char **envp)
{
#ifndef WIN32
  char *path = realpath(argv[0], 0);

  // Try a few times since this may fail because we have an incomplete
  // binary...
  for (int i = 0; i < 5; ++i) {
    int result = execve(path, argv, envp);
    if (result != 0)
#ifndef WIN32
      sleep(1);
#else
      Sleep(1000);
#endif
  }
  perror("execve");
#endif
}

void WServer::handleRequest(WebRequest *request)
{
  impl_->webController_->handleRequest(request);
}

int WServer::waitForShutdown(const char *restartWatchFile)
{
#if !defined(WIN32)
  if (gdb) {
    for(;;)
      sleep(0x1<<16);
  }
#endif // WIN32

#ifdef WT_THREADED

#if !defined(_WIN32)
  sigset_t wait_mask;
  sigemptyset(&wait_mask);

  sigaddset(&wait_mask, SIGHUP);
  sigaddset(&wait_mask, SIGINT);
  sigaddset(&wait_mask, SIGQUIT);
  sigaddset(&wait_mask, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &wait_mask, 0);

#ifdef RESTART_WATCH_FILE
  struct stat st;
  time_t mtime = 0;
  if (restartWatchFile && (stat(restartWatchFile, &st) == 0))
    mtime = st.st_mtime;
#endif // RESTART_WATCH_FILE

  for (;;) {
    int sig;
#ifdef RESTART_WATCH_FILE
    if (mtime) {
      struct timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = 100*1000;
      sig = sigtimedwait(&wait_mask, 0, &ts);
    } else
#endif // RESTART_WATCH_FILE
      sigwait(&wait_mask, &sig);

    if (sig != -1)
      return sig;
#ifdef RESTART_WATCH_FILE
    else
      if (errno != EAGAIN && errno != EINTR) {
	perror("sigtimedwait");
	return -1;
      } else if (errno == EAGAIN && mtime) {
	if (stat(restartWatchFile, &st) == 0)
	  if (st.st_mtime != mtime)
	    return SIGHUP;
      }
#endif // RESTART_WATCH_FILE
  }

#else  // WIN32

  boost::mutex::scoped_lock terminationLock(terminationMutex);
  SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
  ctrlCHit.wait(terminationLock);
  SetConsoleCtrlHandler(console_ctrl_handler, FALSE);
  return 0;

#endif // WIN32
#else
  return 0;
#endif // WT_THREADED
}

void WServer::expireSessions()
{
  impl_->webController_->expireSessions();
}

std::string WServer::appRoot() const
{
  impl_->readConfiguration(this);

  return impl_->webController_->configuration().appRoot();
}

bool WServer::readConfigurationProperty(const std::string& name,
    std::string& value) const
{
  impl_->readConfiguration(this);

  return impl_->webController_->configuration()
    .readConfigurationProperty(name, value);
}

int WRun(int argc, char *argv[], ApplicationCreator createApplication)
{
  try {
    WServer server(argv[0], "");
    try {
      server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
      server.addEntryPoint(Application, createApplication);
      if (server.start()) {
#ifdef WT_THREADED
	int sig = WServer::waitForShutdown(argv[0]);
	server.impl()->serverConfiguration_->log("notice")
	  << "Shutdown (signal = " << sig << ")";
#endif
	server.stop();

#ifdef WT_THREADED
#ifndef WIN32
	if (sig == SIGHUP)
	  // Mac OSX: _NSGetEnviron()
	  WServer::restart(argc, argv, 0);
#endif // WIN32
#endif // WT_THREADED
      }

      return 0;
    } catch (std::exception& e) {
      if (server.impl()->serverConfiguration_)
	server.impl()->serverConfiguration_->log("fatal") << e.what();
      else
	std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (std::exception& e) {
    std::cerr << "exception: " << e.what() << std::endl;
    return 1;
  }
}

}
