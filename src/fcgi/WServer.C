// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WIOService"
#include "Wt/WServer"

#include <iostream>
#include <string>
#include <signal.h>

#include "Configuration.h"
#include "FCGIStream.h"
#include "Server.h"
#include "WebController.h"
#include "WebMain.h"

namespace {
  Wt::WebMain *webMainInstance = 0;
}

namespace Wt {

LOGGER("WServer/wtfcgi");

struct WServer::Impl
{
  Impl(WServer& server)
    : server_(server),
      running_(false),
      webMain_(0)
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
      webMainInstance = webMain_ = 0;

    } catch (std::exception& e) {
      LOG_ERROR_S(&server_,
		  "fatal: dedicated session process for " << sessionId_ <<
		  ": caught unhandled exception: " << e.what());

      throw;
    } catch (...) {
      LOG_ERROR_S(&server_,
		  "fatal: dedicated session process for " << sessionId_ <<
		  ": caught unkown, unhandled exception.");

      throw;
    }
  }

  void startSharedProcess()
  {
    Configuration& conf = server_.configuration();
    
    if (!Server::bindUDStoStdin(conf.runDirectory() + "/server-"
				+ boost::lexical_cast<std::string>(getpid()),
				server_))
      exit(1);

    try {
      FCGIStream fcgiStream;
      webMainInstance = webMain_ = new WebMain(&server_, &fcgiStream);
      webMain_->run();

      delete webMain_;
      webMainInstance = webMain_ = 0;
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
  bool isRelayServer = argc < 2 || strcmp(argv[1], "client") != 0; 

  if (isRelayServer) {
    LOG_INFO_S(this, "initializing relay server");
    Server relayServer(*this, argc, argv);
    exit(relayServer.run());
  } else {
    if (argc >= 3)
      impl_->sessionId_ = argv[2];
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

  if (configuration().webSockets()) {
    LOG_ERROR_S(this, "FastCGI does not support web-sockets, disabling");
    configuration().setWebSockets(false);
  }

  configuration().setNeedReadBodyBeforeResponse(true);

  if (signal(SIGTERM, Wt::handleSigTerm) == SIG_ERR)
    LOG_ERROR_S(this, "cannot catch SIGTERM: signal(): " << strerror(errno));
  if (signal(SIGUSR1, Wt::handleSigUsr1) == SIG_ERR) 
    LOG_ERROR_S(this, "cannot catch SIGUSR1: signal(): " << strerror(errno));
  if (signal(SIGHUP, Wt::handleSigHup) == SIG_ERR) 
    LOG_ERROR_S(this, "cannot catch SIGHUP: signal(): " << strerror(errno));

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

void WServer::setSslPasswordCallback(
  boost::function<std::string (std::size_t max_length, int purpose)> cb)
{
  LOG_INFO_S(this,
    "setSslPasswordCallback(): has no effect in fcgi connector");
}

int WRun(int argc, char *argv[], ApplicationCreator createApplication)
{
  try {
    WServer server(argv[0], "");

    try {
      server.setServerConfiguration(argc, argv);
      server.addEntryPoint(Application, createApplication);
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
