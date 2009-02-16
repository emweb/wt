// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WServer"

#include <iostream>
#include <string>

#if !defined(_WIN32)
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#endif // !_WIN32

#ifdef THREADED
#ifdef BOOST_ASIO
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#else  // BOOST_ASIO
#include <asio/thread.hpp>
#endif // BOOST_ASIO
#endif // THREADED

#include <boost/bind.hpp>

#include "Connection.h"
#include "Server.h"
#include "Configuration.h"
#include "../web/Configuration.h"
#include "WebController.h"
#include "HTTPStream.h"

#ifdef THREADED
#ifdef BOOST_ASIO
typedef boost::thread thread_t;
#else
typedef asio::thread thread_t;
#endif
#endif
namespace {
  static std::string getWtConfigXml(int argc, char *argv[])
  {
    std::string wt_config_xml;
    Wt::WLogger stderrLogger;
    stderrLogger.setStream(std::cerr);
    
    http::server::Configuration serverConfiguration(stderrLogger, true);
    serverConfiguration.setOptions(argc, argv, WTHTTP_CONFIGURATION);
    
    return serverConfiguration.configPath();
  }

}

namespace Wt {

struct WServerImpl {
  WServerImpl(const std::string& wtApplicationPath,
	      const std::string& wtConfigurationFile)
  : wtConfiguration_(wtApplicationPath, wtConfigurationFile,
		     Wt::Configuration::WtHttpdServer,
		     "Wt: initializing built-in httpd"),
    webController_(wtConfiguration_, stream_),
    serverConfiguration_(wtConfiguration_.logger()),
    server_(0)
  { }

  Configuration wtConfiguration_;
  HTTPStream    stream_;
  WebController webController_;

  http::server::Configuration  serverConfiguration_;
  http::server::Server        *server_;
#ifdef THREADED
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
  impl_->serverConfiguration_.setOptions(argc, argv, serverConfigurationFile);
}

void WServer::addEntryPoint(EntryPointType type, ApplicationCreator callback,
			    const std::string& path)
{
  switch (type) {
  case Application:
    impl_->wtConfiguration_.addEntryPoint
      (EntryPoint(WebSession::Application, callback, path));
    break;
  case WidgetSet:
    impl_->wtConfiguration_.addEntryPoint
      (EntryPoint(WebSession::WidgetSet, callback, path));
    break;
  }
}

bool WServer::start()
{
  if (isRunning()) {
    std::cerr << "WServer::start() error: server already started!" << std::endl;
    return false;
  }

#ifndef WIN32
  srand48(getpid());
#endif

  // Override sessionIdPrefix setting
  if (!impl_->serverConfiguration_.sessionIdPrefix().empty())
    impl_->wtConfiguration_.setSessionIdPrefix
      (impl_->serverConfiguration_.sessionIdPrefix());

    // Set default entry point
  impl_->wtConfiguration_.setDefaultEntryPoint
    (impl_->serverConfiguration_.deployPath().substr(1));

  try {
    impl_->server_ = new http::server::Server(impl_->serverConfiguration_,
					      impl_->wtConfiguration_);
#ifndef THREADED
    impl_->serverConfiguration_.log("warn")
      << "No boost thread support, running in main thread.";

    impl_->server_->run();

    delete impl_->server_;
    impl_->server_ = 0;

    return false;
#else // THREADED

#if !defined(_WIN32)
    // Block all signals for background threads.
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif // _WIN32

    int NUM_THREADS = impl_->serverConfiguration_.threads();

    impl_->threads_ = new thread_t *[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i)
      impl_->threads_[i] =
	new thread_t(boost::bind(&http::server::Server::run, impl_->server_));

#if !defined(_WIN32)
    // Restore previous signals.
    pthread_sigmask(SIG_SETMASK, &old_mask, 0);
#endif // _WIN32

#endif // THREADED

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

#if defined(_WIN32) && defined(THREADED)

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

#ifdef THREADED
  try {
    // Stop the Wt application server (cleaning up all sessions).
    impl_->webController_.forceShutdown();

    // Stop the server.
    impl_->server_->stop();

    int NUM_THREADS = impl_->serverConfiguration_.threads();
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

#else // THREADED
  impl_->webController_.forceShutdown();
  impl_->server_->stop();
#endif // THREADED
}

int WServer::waitForShutdown()
{
#ifdef THREADED

#if !defined(_WIN32)

  sigset_t wait_mask;
  sigemptyset(&wait_mask);
  sigaddset(&wait_mask, SIGINT);
  sigaddset(&wait_mask, SIGQUIT);
  sigaddset(&wait_mask, SIGTERM);
  //sigaddset(&wait_mask, SIGCHLD);
  pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
  int sig = 0;
  //sigdelset(&wait_mask, SIGCHLD);
    
  int err;
  do {
    err = sigwait(&wait_mask, &sig);
  } while (err != 0);

  return sig;

#else  // WIN32

  boost::mutex::scoped_lock terminationLock(terminationMutex);
  SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
  ctrlCHit.wait(terminationLock);
  SetConsoleCtrlHandler(console_ctrl_handler, FALSE);
  return 0;

#endif // WIN32

#endif // THREADED
}

int WRun(int argc, char *argv[], ApplicationCreator createApplication)
{
  try {
    WServer server(argv[0], getWtConfigXml(argc, argv));
    try {
      server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
      server.addEntryPoint(WServer::Application, createApplication);
      if (server.start()) {
	int sig = WServer::waitForShutdown();
	server.impl()->serverConfiguration_.log("notice")
	  << "Shutdown (signal = " << sig << ")";
	server.stop();
      }

      return 0;
    } catch (std::exception& e) {
      server.impl()->serverConfiguration_.log("fatal") << e.what();
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
