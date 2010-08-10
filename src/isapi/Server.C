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
  terminated_(false)
{
  serverThread_ = boost::thread(boost::bind(&IsapiServer::serverEntry, this));
}

IsapiServer::~IsapiServer()
{
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
  } catch (std::exception &e) {
    terminationMsg = uncaughtExceptionReply;
    // TODO: log
  } catch(...) {
    terminationMsg = uncaughtExceptionReply;
    // TODO: log
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
  {
    boost::mutex::scoped_lock l(queueMutex_);
    server_->stop();
  }
  serverThread_.join();
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

/*
 ******************
 * Client routines
 ******************
 */

#if 0
static void doShutdown(const char *signal)
{
  unlink(socketPath.c_str());
  if (theController) {
    theController->configuration().log("notice") << "Caught " << signal;
    theController->forceShutdown();
  }

  exit(0);
}
#endif
}
static WebController *theController = 0;

void startSharedProcess(Configuration& conf, WServer *server)
{


}


struct WServerImpl {
  WServerImpl(const std::string& applicationPath,
	      const std::string& configurationFile)
    : applicationPath_(applicationPath),
      configurationFile_(configurationFile),
      configuration_(0),
      running_(false)
      //,
      //relayServer_(0),
  { }

  std::string applicationPath_;
  std::string configurationFile_;
  Configuration *configuration_;
  bool          running_;
#if 0
  Server        *relayServer_;
  std::string   sessionId_;
#endif
};

WServer::WServer(const std::string& applicationPath,
		 const std::string& configurationFile)
  : impl_(new WServerImpl(applicationPath, configurationFile))
{
  if (!isapi::IsapiServer::instance()->addServer(this))
    throw Exception("WServer::WServer(): "
		    "Only one simultaneous WServer supported");
}

WServer::~WServer()
{
  delete impl_;
}

void WServer::setServerConfiguration(int argc, char *argv[],
				     const std::string& serverConfigurationFile)
{
  bool isServer = argc < 2 || strcmp(argv[1], "client") != 0; 

  std::string approot;
  {
    char buffer[1024];
    GetPrivateProfileString("isapi", "approot", "",
      buffer, sizeof(buffer), (std::string(argv[0]) + ".ini").c_str());
    approot = buffer;
  }

  impl_->configuration_ = new Configuration(impl_->applicationPath_,
    approot,
    impl_->configurationFile_,
    Configuration::IsapiServer,
    "Wt: initializing session process");
}

void WServer::addEntryPoint(EntryPointType type, ApplicationCreator callback,
			    const std::string& path, const std::string& favicon)
{
  if (!impl_->configuration_)
    throw Exception("WServer::addEntryPoint(): "
		    "call setServerConfiguration() first");

  impl_->configuration_
    ->addEntryPoint(EntryPoint(type, callback, path, favicon));
}

bool WServer::start()
{
  if (!impl_->configuration_)
    throw Exception("WServer::start(): call setServerConfiguration() first");

  if (isRunning()) {
    std::cerr << "WServer::start() error: server already started!" << std::endl;
    return false;
  }

  impl_->running_ = true;

  try {
    isapi::IsapiStream isapiStream(isapi::IsapiServer::instance());
    WebController controller(*impl_->configuration_, this, &isapiStream);
    theController = &controller;

    controller.run();

    theController = 0;

  } catch (std::exception& e) {
    impl_->configuration_->log("fatal")
      << "ISAPI server: caught unhandled exception: " << e.what();

    throw;
  } catch (...) {
    impl_->configuration_->log("fatal")
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
  impl()->configuration_->log("error")
    << "WServer::addResource() not supported by ISAPI connector.";
}

void WServer::handleRequest(WebRequest *request)
{
  theController->handleRequest(request);
}

std::string WServer::approot() const
{
  return impl_->configuration_->approot();
}

bool WServer::readConfigurationProperty(const std::string& name,
                                        std::string& value) const
{
  return impl_->configuration_->readConfigurationProperty(name, value);
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
      server.impl()->configuration_->log("fatal") << e.what();
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
