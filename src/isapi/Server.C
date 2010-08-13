/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Server.h"
#include "Configuration.h"
#include "IsapiRequest.h"
#include "IsapiStream.h"

#include <windows.h>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <exception>
#include <vector>

#include "WebController.h"

#include "Wt/WResource"
#include "Wt/WServer"
#include "Wt/WLogger"

using std::exit;
using std::strcpy;
using std::strlen;
using std::memset;
extern int main(int argc, char **argv);

namespace {
  const char *terminationMsg = 0;
  const char *mainReturnedReply =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>Internal Server Error</h1>Wt-ISAPI terminated: returned from main</body>"
    "</html>";
  const char *uncaughtExceptionReply =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>Internal Server Error</h1>Wt-ISAPI terminated: uncaught exception</body>"
    "</html>";
}

namespace Wt {
  namespace isapi {

IsapiServer *IsapiServer::instance_;

IsapiServer::IsapiServer():
  server_(0),
  terminated_(false),
  configuration_(0)
{
  serverThread_ = boost::thread(boost::bind(&IsapiServer::serverEntry, this));
}

IsapiServer::~IsapiServer()
{
  delete configuration_;
}

namespace {
  HMODULE GetCurrentModule()
  {
    HMODULE hModule = 0;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
      (LPCTSTR)GetCurrentModule, &hModule);

    return hModule;
  }
}

void IsapiServer::serverEntry() {
  HMODULE module = GetCurrentModule();
  char DllPath[_MAX_PATH];
  char *pDllPath = DllPath;
  GetModuleFileName(module, DllPath, _MAX_PATH);
  FreeLibrary(module);

  try {
    main(1, &pDllPath);
    terminationMsg = mainReturnedReply;
    if (configuration())
      log("fatal") << "ISAPI: main() returned";
  } catch (std::exception &e) {
    terminationMsg = uncaughtExceptionReply;
    if (configuration())
      log("fatal") << "ISAPI: uncaught main() exception: " << e.what();
  } catch(...) {
    terminationMsg = uncaughtExceptionReply;
    if (configuration())
      log("fatal") << "ISAPI: unknown uncaught main() exception";
  }
  setTerminated();
}

void IsapiServer::pushRequest(IsapiRequest *request) {
  if (request->isGood()) {
    boost::mutex::scoped_lock l(queueMutex_);
    if (!terminated_) {
      queue_.push_back(request);
      queueCond_.notify_all();
    } else {
      request->sendSimpleReply(500, terminationMsg);
    }
  } else {
    // incomplete request received
    request->abort();
    delete request;
  }
}

IsapiRequest *IsapiServer::popRequest(int timeoutSec)
{
  boost::system_time const deadline =
    boost::get_system_time() + boost::posix_time::seconds(timeoutSec);
  boost::mutex::scoped_lock l(queueMutex_);
  while (true) {
    if (queue_.size()) {
      IsapiRequest *retval = queue_.front();
      queue_.pop_front();
      return retval;
    } else {
      // Wait until an element is inserted in the queue...
      if (!queueCond_.timed_wait(l, deadline)) {
        // timeout
        return 0;
      }
    }
  }
  return 0;
}

void IsapiServer::setTerminated()
{
  boost::mutex::scoped_lock l(queueMutex_);
  terminated_ = true;
  while (queue_.size()) {
    IsapiRequest *retval = queue_.front();
    queue_.pop_front();
    l.unlock();
    retval->sendSimpleReply(500, terminationMsg);
    l.lock();
  }
}

void IsapiServer::shutdown()
{
  if (configuration())
    log("notice") << "ISAPI: shutdown requested...";
  {
    boost::mutex::scoped_lock l(queueMutex_);
    server_->stop();
  }
  serverThread_.join();
  if (configuration())
    log("notice") << "ISAPI: shutdown completed...";
}

IsapiServer *IsapiServer::instance()
{
  if (!instance_) {
    instance_ = new IsapiServer();
  }
  return instance_;
}

bool IsapiServer::addServer(WServer *server)
{
  boost::mutex::scoped_lock l(queueMutex_);
  if (server_) return false;
  server_ = server;
  return true;
}

void IsapiServer::removeServer(WServer *server)
{
  boost::mutex::scoped_lock l(queueMutex_);
  if (server_ != server) {
    if (configuration()) {
      log("error") << "ISAPI internal error: removeServer() inconsistent";
    }
  }
  server_ = 0;
}

WLogEntry IsapiServer::log(const std::string& type)
{
  return configuration()->log(type);
}

}

static WebController *theController = 0;

struct WServerImpl {
  WServerImpl()
    : running_(false)
  { }

  Configuration *configuration() {
    return isapi::IsapiServer::instance()->configuration();
  }
  bool running_;
};

WServer::WServer(const std::string& applicationPath,
		 const std::string& configurationFile)
  : impl_(new WServerImpl())
{
  if (!isapi::IsapiServer::instance()->addServer(this))
    throw Exception("WServer::WServer(): "
		    "Only one simultaneous WServer supported");

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

  isapi::IsapiServer::instance()->setConfiguration(
    new Configuration(applicationPath, approot, configurationFile,
      Configuration::IsapiServer, "Wt: initializing session process"));

  if (approotLog.str() != "") {
    impl_->configuration()->log("notice") << approotLog.str();
  }
}

WServer::~WServer()
{
  isapi::IsapiServer::instance()->removeServer(this);
  delete impl_;
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string& serverConfigurationFile)
{

}

void WServer::addEntryPoint(EntryPointType type, ApplicationCreator callback,
			    const std::string& path, const std::string& favicon)
{
  if (!impl_->configuration())
    throw Exception("WServer::addEntryPoint(): "
		    "call setServerConfiguration() first");

  impl_->configuration()
    ->addEntryPoint(EntryPoint(type, callback, path, favicon));
}

bool WServer::start()
{
  if (!impl_->configuration())
    throw Exception("WServer::start(): call setServerConfiguration() first");

  if (isRunning()) {
    impl_->configuration()->log("error")
      << "WServer::start() error: server already started!";
    return false;
  }

  impl_->running_ = true;

  try {
    isapi::IsapiStream isapiStream(isapi::IsapiServer::instance());
    WebController controller(*impl_->configuration(), this, &isapiStream);
    theController = &controller;

    controller.run();

    theController = 0;

  } catch (std::exception& e) {
    impl_->configuration()->log("fatal")
      << "ISAPI server: caught unhandled exception: " << e.what();

    throw;
  } catch (...) {
    impl_->configuration()->log("fatal")
      << "ISAPI server: caught unknown, unhandled exception.";
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
  theController->forceShutdown();
}

int WServer::waitForShutdown(const char *restartWatchFile)
{
  for (;;)
    Sleep(10000 * 1000);

  return 0;
}

void WServer::addResource(WResource *resource, const std::string& path)
{
  if (!boost::starts_with(path, "/")) 
    throw WServer::Exception("WServer::addResource() error: "
			     "static resource path should start with \'/\'");

  resource->setInternalPath(path);
  impl_->configuration()->addEntryPoint(EntryPoint(resource, path));
}

void WServer::handleRequest(WebRequest *request)
{
  theController->handleRequest(request);
}

std::string WServer::appRoot() const
{
  return impl_->configuration()->appRoot();
}

bool WServer::readConfigurationProperty(const std::string& name,
                                        std::string& value) const
{
  return impl_->configuration()->readConfigurationProperty(name, value);
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
      server.impl()->configuration()->log("fatal") << e.what();
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
