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

#include "Configuration.h"
#include "FCGIStream.h"
#include "Server.h"
#include "WebController.h"
#include "WebMain.h"

namespace {

  Wt::WebMain *webMainInstance = 0;

}

namespace Wt {

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
      server_.log("fatal") << "wtfcgi dedicated session process for "
			   << sessionId_
			   << ": caught unhandled exception: " << e.what();

      throw;
    } catch (...) {
      server_.log("fatal") << "wtfcgi dedicated session process for "
			   << sessionId_
			   << ": caught unkown, unhandled exception.";

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
      server_.log("fatal") << "wtfcgi shared session process: "
			   << "caught unhandled exception: " << e.what();

      throw;
    } catch (...) {
      server_.log("fatal") << "wtfcgi shared session process: "
			   << "caught unknown, unhandled exception.";

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
    webMainInstance->controller().server()->log("notice")
      << "Caught " << signal;
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

WServer::~WServer()
{
  delete impl_;

  destroy();
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string&)
{
  bool isRelayServer = argc < 2 || strcmp(argv[1], "client") != 0; 

  if (isRelayServer) {
    log("notice") << "Wt: initializing wtfcgi relay server";
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
    log("error") << "WServer::start(): server already started!";
    return false;
  }

  log("notice") << "Wt: initializing "
		<< (impl_->sessionId_.empty() ? "shared" : "dedicated")
		<< " wtfcgi session process";

  if (configuration().webSockets()) {
    log("error") << "FastCGI does not support web-sockets, disabling";
    configuration().setWebSockets(false);
  }

  configuration().setNeedReadBodyBeforeResponse(true);

  if (signal(SIGTERM, Wt::handleSigTerm) == SIG_ERR)
    log("error") << "Cannot catch SIGTERM: signal(): " << strerror(errno);
  if (signal(SIGUSR1, Wt::handleSigUsr1) == SIG_ERR) 
    log("error") << "Cannot catch SIGUSR1: signal(): " << strerror(errno);
  if (signal(SIGHUP, Wt::handleSigHup) == SIG_ERR) 
    log("error") << "Cannot catch SIGHUP: signal(): " << strerror(errno);

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
    log("error") << "WServer::stop(): server not yet started!";
    return;
  }
}

void WServer::resume()
{
  if (!isRunning()) {
    log("error") << "WServer::resume(): server not yet started!";
    return;
  }
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
