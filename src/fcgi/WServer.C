// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WIOService.h"
#include "Wt/WServer.h"

#include <iostream>
#include <string>
#include <signal.h>

#include "Configuration.h"
#include "FCGIStream.h"
#include "Server.h"
#include "WebController.h"
#include "WebMain.h"

namespace {
  Wt::WebMain *webMainInstance = nullptr;
}

namespace Wt {

LOGGER("WServer/wtfcgi");

struct WServer::Impl
{
  Impl(WServer& server)
    : server_(server),
      running_(false),
      webMain_(nullptr)
  { }

  void run()
  {
    running_ = true;

    if (sessionId_.empty())
      startSharedProcess();
    else
      runSession();
  }

  void runSession()
  {
    Configuration& conf = server_.configuration();

    if (!Server::bindUDStoStdin(conf.runDirectory() + "/" + sessionId_,
				server_))
      exit(1);

    try {
      FCGIStream fcgiStream;
      webMainInstance = webMain_
	= new WebMain(&server_, &fcgiStream, sessionId_);
      webMain_->run();

      sleep(1);

      delete webMain_;
      webMainInstance = webMain_ = nullptr;

    } catch (std::exception& e) {
      LOG_ERROR_S(&server_,
		  "fatal: dedicated session process for " << sessionId_ <<
		  ": caught unhandled exception: " << e.what());

      throw;
    } catch (...) {
      LOG_ERROR_S(&server_,
		  "fatal: dedicated session process for " << sessionId_ <<
		  ": caught unknown, unhandled exception.");

      throw;
    }
  }

  void startSharedProcess()
  {
    Configuration& conf = server_.configuration();
    
    if (!Server::bindUDStoStdin(conf.runDirectory() + "/server-"
				+ std::to_string(getpid()),
				server_))
      exit(1);

    try {
      FCGIStream fcgiStream;
      webMainInstance = webMain_ = new WebMain(&server_, &fcgiStream);
      webMain_->run();

      delete webMain_;
      webMainInstance = webMain_ = nullptr;
    } catch (std::exception& e) {
      LOG_ERROR_S(&server_,
		  "fatal: shared session process: caught unhandled exception: "
		  << e.what());

      throw;
    } catch (...) {
      LOG_ERROR_S(&server_,
		  "fatal: shared session process: caught unknown, unhandled "
		  "exception.");

      throw;
    }
  }

  WServer& server_;
  bool running_;
  std::string sessionId_;
  WebMain *webMain_;
};

namespace {

void doShutdown(const char *signal)
{
  if (webMainInstance) {
    LOG_INFO_S(webMainInstance->controller().server(), "Caught " << signal);
    webMainInstance->shutdown();
  }
}

void handleSigTerm(int)
{
  doShutdown("SIGTERM");
}

void handleSigUsr1(int)
{
  doShutdown("SIGUSR1");
}

void handleSigHup(int)
{
  doShutdown("SIGHUP");
}

}

WServer::WServer(const std::string& applicationPath,
		 const std::string& wtConfigurationFile)
  : impl_(new Impl(*this))
{ 
  init(applicationPath, wtConfigurationFile);
}

WServer::WServer(int argc, char *argv[], const std::string& wtConfigurationFile)
  : impl_(new Impl(*this))
{
  init(argv[0], "");

  setServerConfiguration(argc, argv, wtConfigurationFile);
}

WServer::WServer(const std::string &applicationPath,
                 const std::vector<std::string> &args,
                 const std::string &wtConfigurationFile)
  : impl_(new Impl(*this))
{
  init(applicationPath, "");

  setServerConfiguration(applicationPath, args, wtConfigurationFile);
}

WServer::~WServer()
{
  delete impl_;
  destroy();
}

std::vector<WServer::SessionInfo> WServer::sessions() const
{
  return std::vector<WServer::SessionInfo>();
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string&)
{
  std::string applicationPath = argv[0];
  std::vector<std::string> args(argv + 1, argv + argc);

  setServerConfiguration(applicationPath, args);
}

void WServer::setServerConfiguration(const std::string &applicationPath,
                                     const std::vector<std::string> &args,
                                     const std::string &wtConfigurationFile)
{
  bool isRelayServer = args.size() < 1 || args[0] != "client";

  if (isRelayServer) {
    LOG_INFO_S(this, "initializing relay server");
    Server relayServer(*this, applicationPath, args);
    exit(relayServer.run());
  } else {
    if (args.size() >= 2)
      impl_->sessionId_ = args[1];
  }
}

bool WServer::start()
{
  if (isRunning()) {
    LOG_ERROR_S(this, "start(): server already started!");
    return false;
  }

  LOG_INFO_S(this, "initializing " <<
	     (impl_->sessionId_.empty() ? "shared" : "dedicated") <<
	     " wtfcgi session process");

  dedicatedProcessEnabled_ = !impl_->sessionId_.empty();

  if (configuration().webSockets()) {
    LOG_ERROR_S(this, "FastCGI does not support web-sockets, disabling");
    configuration().setWebSockets(false);
  }

  configuration().setNeedReadBodyBeforeResponse(true);

  if (signal(SIGTERM, Wt::handleSigTerm) == SIG_ERR)
    LOG_ERROR_S(this, "cannot catch SIGTERM: signal(): "
		<< (const char *)strerror(errno));
  if (signal(SIGUSR1, Wt::handleSigUsr1) == SIG_ERR) 
    LOG_ERROR_S(this, "cannot catch SIGUSR1: signal(): "
		<< (const char *)strerror(errno));
  if (signal(SIGHUP, Wt::handleSigHup) == SIG_ERR) 
    LOG_ERROR_S(this, "cannot catch SIGHUP: signal(): "
		<< (const char *)strerror(errno));

  webController_ = new Wt::WebController(*this, impl_->sessionId_, false);

  impl_->run();

  return false;
}

bool WServer::isRunning() const
{
  return impl_->running_;
}

int WServer::httpPort() const
{
  // FIXME, we could get this from the CGI environment.
  return -1;
}

void WServer::stop()
{
  if (!isRunning()) {
    LOG_ERROR_S(this, "stop(): server not yet started!");
    return;
  }
}

void WServer::run()
{
  if (start()) {
    waitForShutdown();
    stop();
  }
}

void WServer::resume()
{
  if (!isRunning()) {
    LOG_ERROR_S(this, "resume(): server not yet started!");
    return;
  }
}

void WServer::setSslPasswordCallback(const std::function<std::string 
			 (std::size_t max_length, int purpose)>& cb)
{
  LOG_INFO_S(this,
    "setSslPasswordCallback(): has no effect in fcgi connector");
}

int WRun(int argc, char *argv[], ApplicationCreator createApplication)
{
  std::string applicationPath = argv[0];
  std::vector<std::string> args(argv + 1, argv + argc);

  return WRun(applicationPath, args, createApplication);
}

int WRun(const std::string &applicationName,
         const std::vector<std::string> &args,
         ApplicationCreator createApplication)
{
  try {
    WServer server(applicationName, "");

    try {
      server.setServerConfiguration(applicationName, args);
      server.addEntryPoint(EntryPointType::Application, createApplication);
      server.start();

      return 0;
    } catch (std::exception& e) {
      LOG_ERROR_S(&server, "fatal: " << e.what());
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
