/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WServer"
#include "IsapiStream.h"
#include "Server.h"
#include "Wt/WLogger"
#include "WebMain.h"
#include "WebController.h"

#include <boost/algorithm/string.hpp>

#include <Windows.h>
#include <fstream>

namespace Wt {

static WebMain *webMain = 0;

struct WServer::Impl {
  Impl()
    : running_(false)
  { }

  void init(WServer *server, const std::string& applicationPath, const std::string& configurationFile)
  {
    if (!isapi::IsapiServer::instance()->addServer(server))
      throw Exception("WServer::WServer(): "
		      "Only one simultaneous WServer supported");
    server->instance_ = server;

    std::stringstream approotLog;
    std::string approot;
    {
      std::string inifile = applicationPath + ".ini";
      char buffer[1024];
      GetPrivateProfileString("isapi", "approot", "",
	buffer, sizeof(buffer), inifile.c_str());
      approot = buffer;
      if (approot != "") {
	approotLog << "ISAPI: read approot (" << approot
	  << ") from ini file " << inifile;
      }
    }

    server->setAppRoot(approot);
    server->setConfiguration(configurationFile);
    if (approotLog.str() != "") {
      server->log("info") << approotLog.str();
    }
  }

  bool running_;
};

WServer::WServer(const std::string& applicationPath,
		 const std::string& configurationFile)
  : impl_(new Impl())
{
  init(applicationPath, configurationFile);
  impl_->init(this, applicationPath, configurationFile);
}

WServer::WServer(int argc, char *argv[], const std::string& wtConfigurationFile)
  : impl_(new Impl())
{
  init(argv[0], wtConfigurationFile);
  impl_->init(this, argv[0], wtConfigurationFile);
  // setServerConfiguration doesn't actually do anything, but I'll
  // just call it here just in case it starts doing anything.
  setServerConfiguration(argc, argv, wtConfigurationFile);
}

WServer::~WServer()
{
  isapi::IsapiServer::instance()->removeServer(this);
  delete impl_;
}

std::vector<WServer::SessionInfo> WServer::sessions() const
{
  return std::vector<WServer::SessionInfo>();
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string& serverConfigurationFile)
{

}

//void WServer::addEntryPoint(EntryPointType type, ApplicationCreator callback,
//			    const std::string& path, const std::string& favicon)
//{
//  if (!impl_->configuration())
//    throw Exception("WServer::addEntryPoint(): "
//		    "call setServerConfiguration() first");
//
//  impl_->configuration()
//    ->addEntryPoint(EntryPoint(type, callback, path, favicon));
//}

bool WServer::start()
{
  if (isRunning()) {
    log("error") << "WServer::start() error: server already started!";
    return false;
  }

  impl_->running_ = true;
  webController_ = new Wt::WebController(*this, "", false);

  try {
    isapi::IsapiStream isapiStream(isapi::IsapiServer::instance());

    WebMain requestServer(this, &isapiStream);
    webMain = &requestServer;

    isapi::IsapiServer::instance()->setServerStarted();

    requestServer.run();

    webMain = 0;

  } catch (std::exception& e) {
    log("fatal") << "ISAPI server: caught unhandled exception: " << e.what();

    throw;
  } catch (...) {
    log("fatal") << "ISAPI server: caught unknown, unhandled exception.";
    throw;
  }

  return true;
}

bool WServer::isRunning() const
{
  return impl_ && impl_->running_;
}

int WServer::httpPort() const
{
  return -1;
}

void WServer::stop()
{
  if (!isRunning()) {
    std::cerr << "WServer::stop() error: server not yet started!" << std::endl;
    return;
  }
  webMain->shutdown();
}

void WServer::run()
{
  if (start()) {
    waitForShutdown();
    stop();
  }
}

//int WServer::waitForShutdown(const char *restartWatchFile)
//{
//  for (;;)
//    Sleep(10000 * 1000);
//
//  return 0;
//}

//void WServer::handleRequest(WebRequest *request)
//{
//  webMain->controller().handleRequest(request);
//}

//void WServer::schedule(int milliSeconds,
//		       const boost::function<void ()>& function)
//{
//  webMain->schedule(milliSeconds, function);
//}

//void WServer::post(const std::string& sessionId,
//		   const boost::function<void ()>& function,
//		   const boost::function<void ()>& fallbackFunction)
//{
//  schedule(0, sessionId, function, fallbackFunction);
//}

//void WServer::schedule(int milliSeconds,
//		       const std::string& sessionId,
//		       const boost::function<void ()>& function,
//		       const boost::function<void ()>& fallbackFunction)
//{
//  ApplicationEvent event(sessionId, function, fallbackFunction);
//
//  schedule(milliSeconds, boost::bind(&WebController::handleApplicationEvent,
//				     &webMain->controller(), event));
//}

//std::string WServer::appRoot() const
//{
//  return impl_->configuration()->appRoot();
//}

//void WServer::initializeThread()
//{ }

//bool WServer::usesSlashExceptionForInternalPaths() const
//{
//  // Is not relevent, one cannot even deploy at a path ending with '/' ?
//  return false;
//}

//bool WServer::readConfigurationProperty(const std::string& name,
//                                        std::string& value) const
//{
//  return impl_->configuration()->readConfigurationProperty(name, value);
//}

void WServer::setSslPasswordCallback(
  boost::function<std::string (std::size_t max_length, int purpose)> cb)
{
  log("info") << "setSslPasswordCallback(): has no effect in isapi connector";
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
  } catch (Wt::WServer::Exception&) {
    //std::cerr << e.what() << std::endl;
    return 1;
  } catch (std::exception&) {
    //std::cerr << "exception: " << e.what() << std::endl;
    return 1;
  }
}

}
