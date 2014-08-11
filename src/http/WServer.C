// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/asio.hpp>

#include "Wt/WIOService"
#include "Wt/WServer"

#include <iostream>
#include <string>

#include "Connection.h"
#include "Server.h"
#include "Configuration.h"
#include "../web/Configuration.h"
#include "WebController.h"

#if !defined(WT_WIN32)
#include <signal.h>
#endif

#if defined(WT_WIN32)
#include <process.h>
#endif

#ifdef ANDROID
#include "Android.h"
#endif

namespace {
  static void parseArgsPartially(int argc, char *argv[],
				 const std::string& configurationFile,
				 std::string& wtConfigXml,
				 std::string& appRoot)
  {
    std::string wt_config_xml;
    Wt::WLogger stderrLogger;
    stderrLogger.setStream(std::cerr);
    
    http::server::Configuration serverConfiguration(stderrLogger, true);
    serverConfiguration.setOptions(argc, argv, configurationFile);
    
    wtConfigXml = serverConfiguration.configPath();
    appRoot = serverConfiguration.appRoot();
  }

}

namespace Wt {

LOGGER("WServer/wthttp");

struct WServer::Impl
{
  Impl()
    : serverConfiguration_(0),
      server_(0)
  {
#ifdef ANDROID
    preventRemoveOfSymbolsDuringLinking();
#endif
  }

  ~Impl()
  {
    delete serverConfiguration_;
  }

  http::server::Configuration *serverConfiguration_;
  http::server::Server        *server_;
};

WServer::WServer(const std::string& applicationPath,
		 const std::string& wtConfigurationFile)
  : impl_(new Impl())
{ 
  init(applicationPath, wtConfigurationFile);
}

WServer::WServer(int argc, char *argv[], const std::string& wtConfigurationFile)
  : impl_(new Impl())
{
  init(argv[0], "");

  setServerConfiguration(argc, argv, wtConfigurationFile);
}

WServer::~WServer()
{
  if (impl_->server_) {
    try {
      stop();
    } catch (...) {
      LOG_ERROR("~WServer: oops, stop() threw exception!");
    }
  }

  delete impl_;

  destroy();
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string& serverConfigurationFile)
{
  std::string wtConfigFile, appRoot;

  parseArgsPartially(argc, argv, serverConfigurationFile,
		     wtConfigFile, appRoot);

  if (!appRoot.empty())
    setAppRoot(appRoot);

  if (configurationFile().empty())
    setConfiguration(wtConfigFile);

  webController_ = new Wt::WebController(*this);

  impl_->serverConfiguration_ = new http::server::Configuration(logger());

  impl_->serverConfiguration_->setSslPasswordCallback(sslPasswordCallback_);

  if (argc != 0)
    impl_->serverConfiguration_->setOptions(argc, argv,
					    serverConfigurationFile);
}

bool WServer::start()
{
  setCatchSignals(!impl_->serverConfiguration_->gdb());

  stopCallback_ = boost::bind(&WServer::stop, this);

  if (isRunning()) {
    LOG_ERROR("start(): server already started!");
    return false;
  }

  LOG_INFO("initializing built-in wthttpd");

#ifndef WT_WIN32
  srand48(getpid());
#endif

  // Override configuration settings
  configuration().setRunDirectory(std::string());

  configuration().setUseSlashExceptionForInternalPaths
    (impl_->serverConfiguration_->defaultStatic());
  
  if (!impl_->serverConfiguration_->sessionIdPrefix().empty())
    configuration().setSessionIdPrefix(impl_->serverConfiguration_
				       ->sessionIdPrefix());

  configuration().setDefaultEntryPoint(impl_->serverConfiguration_
				       ->deployPath());

  if (impl_->serverConfiguration_->threads() != -1)
    configuration().setNumThreads(impl_->serverConfiguration_->threads());

  if (impl_->serverConfiguration_->parentPort() != -1) {
    configuration().setBehindReverseProxy(true);
    configuration().setSingleSession(true);
  }

  try {
    impl_->server_ = new http::server::Server(*impl_->serverConfiguration_,
					      *this);

#ifndef WT_THREADED
    LOG_WARN("No boost thread support, running in main thread.");
#endif // WT_THREADED

    webController_->start();

    ioService().start();

#ifndef WT_THREADED
    delete impl_->server_;
    impl_->server_ = 0;

    ioService().stop();

    return false;
#else
    return true;
#endif // WT_THREADED

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

void WServer::resume()
{
  if (!isRunning()) {
    LOG_ERROR("resume(): server not yet started!");
    return;
  } else {
    impl_->server_->resume();    
  }
}

void WServer::stop()
{
  if (!isRunning()) {
    LOG_ERROR("stop(): server not yet started!");
    return;
  }

#ifdef WT_THREADED
  try {
    // Stop the Wt application server (cleaning up all sessions).
    webController_->shutdown();

    LOG_INFO("Shutdown: stopping web server.");

    // Stop the server.
    impl_->server_->stop();

    ioService().stop();

    delete impl_->server_;
    impl_->server_ = 0;
  } catch (asio_system_error& e) {
    throw Exception(std::string("Error (asio): ") + e.what());
  } catch (std::exception& e) {
    throw Exception(std::string("Error: ") + e.what());
  }

#else // WT_THREADED
  webController_->shutdown();
  impl_->server_->stop();
  ioService().stop();
#endif // WT_THREADED
}

void WServer::run()
{
  if (start()) {
    waitForShutdown();
    stop();
  }
}

int WServer::httpPort() const
{
  return impl_->server_->httpPort();
}

std::vector<WServer::SessionInfo> WServer::sessions() const
{
  if (configuration_->sessionPolicy() == Wt::Configuration::DedicatedProcess &&
      impl_->serverConfiguration_->parentPort() == -1) {
    return impl_->server_->sessionManager()->sessions();
  } else {
#ifndef WT_WIN32
    int64_t pid = getpid();
#else // WT_WIN32
    int64_t pid = _getpid();
#endif // WT_WIN32
    std::vector<std::string> sessionIds = webController_->sessions();
    std::vector<WServer::SessionInfo> result;
    for (std::size_t i = 0; i < sessionIds.size(); ++i) {
      SessionInfo sessionInfo;
      sessionInfo.processId = pid;
      sessionInfo.sessionId = sessionIds[i];
      result.push_back(sessionInfo);
    }
    return result;
  }
}

void WServer::setSslPasswordCallback(
  boost::function<std::string (std::size_t max_length, int purpose)> cb)
{
  sslPasswordCallback_ = cb;
  if (impl_ && impl_->serverConfiguration_)
    impl_->serverConfiguration_->setSslPasswordCallback(sslPasswordCallback_);
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
	// MacOSX + valgrind:
	// for (;;) { sleep(100); }
	int sig = WServer::waitForShutdown(argv[0]);
	LOG_INFO_S(&server, "shutdown (signal = " << sig << ")");
#endif
	server.stop();

#ifdef WT_THREADED
#ifndef WT_WIN32
	if (sig == SIGHUP)
	  // Mac OSX: _NSGetEnviron()
	  WServer::restart(argc, argv, 0);
#endif // WIN32
#endif // WT_THREADED
      }

      return 0;
    } catch (std::exception& e) {
      LOG_INFO_S(&server, "fatal: " << e.what());
      return 1;
    }
  } catch (Wt::WServer::Exception& e) {
    LOG_ERROR("fatal: " << e.what());
    return 1;
  } catch (std::exception& e) {
    LOG_ERROR("fatal exception: " << e.what());
    return 1;
  }
}

}
