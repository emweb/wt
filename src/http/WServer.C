// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WIOService"
#include "Wt/WServer"

#include <iostream>
#include <string>

#include "Connection.h"
#include "Server.h"
#include "Configuration.h"
#include "../web/Configuration.h"
#include "WebController.h"

#if !defined(_WIN32)
#include <signal.h>
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

WServer::~WServer()
{
  if (impl_->server_) {
    try {
      stop();
    } catch (...) {
      Wt::log("error") << "~WServer: oops, stop() threw exception!";
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

  if (configurationFile().empty())
    setConfiguration(wtConfigFile);

  webController_ = new Wt::WebController(*this);

  impl_->serverConfiguration_ = new http::server::Configuration(logger());

  if (argc != 0)
    impl_->serverConfiguration_->setOptions(argc, argv,
					    serverConfigurationFile);
}

bool WServer::start()
{
  setCatchSignals(!impl_->serverConfiguration_->gdb());

  if (isRunning()) {
    log("error") << "WServer::start(): server already started!";
    return false;
  }

  log("notice") << "Wt: initializing built-in wthttpd";

#ifndef WIN32
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

  try {
    impl_->server_ = new http::server::Server(*impl_->serverConfiguration_,
					      *this);

#ifndef WT_THREADED
    log("warn") << "No boost thread support, running in main thread.";
#endif // WT_THREADED

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
    log("error") << "WServer::resume(): server not yet started!";
    return;
  } else {
    impl_->server_->resume();    
  }
}

void WServer::stop()
{
  if (!isRunning()) {
    log("error") << "WServer::stop(): server not yet started!";
    return;
  }

#ifdef WT_THREADED
  try {
    // Stop the Wt application server (cleaning up all sessions).
    webController_->shutdown();

    log("notice") << "Shutdown: stopping web server.";

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
#endif // WT_THREADED
}

int WServer::httpPort() const
{
  return impl_->server_->httpPort();
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
	server.log("notice") << "Shutdown (signal = " << sig << ")";
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
      server.log("fatal") << e.what();
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
