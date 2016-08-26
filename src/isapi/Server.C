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

#include "WebMain.h"

#include "Wt/WResource"
#include "Wt/WServer"
#include "Wt/WLogger"

#include <fstream>

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
  boost::mutex::scoped_lock l(startedMutex_);
  started_ = false;
  instance_ = this; // must be set before serverEntry() is called
  serverThread_ = boost::thread(boost::bind(&IsapiServer::serverEntry, this));
  // don't return before WRun was executed and the server is actually running
  while (!started_)
    startedCondition_.wait(l);
}

IsapiServer::~IsapiServer()
{
  //delete configuration_;
}

void IsapiServer::setServerStarted()
{
  // unblock the constructor, ISAPI init function may return
  started_ = true;
  startedCondition_.notify_all();
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
    if (hasConfiguration())
      log("fatal") << "ISAPI: main() returned";
  } catch (std::exception &e) {
    terminationMsg = uncaughtExceptionReply;
    if (hasConfiguration())
      log("fatal") << "ISAPI: uncaught main() exception: " << e.what();
  } catch(...) {
    terminationMsg = uncaughtExceptionReply;
    if (hasConfiguration())
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
  if (hasConfiguration())
    log("notice") << "ISAPI: shutdown requested...";
  {
    boost::mutex::scoped_lock l(queueMutex_);
    server_->stop();
  }
  serverThread_.join();
  if (hasConfiguration())
    log("notice") << "ISAPI: shutdown completed...";
}

IsapiServer *IsapiServer::instance()
{
  if (!instance_) {
    // note: this is too late to set instance_ since instance() is already
    // called from the constructor call graph. instance_ must be set
    // in the constructor.
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
    if (hasConfiguration()) {
      log("error") << "ISAPI internal error: removeServer() inconsistent";
    }
  }
  server_ = 0;
}

WLogEntry IsapiServer::log(const std::string& type)
{
  return server_->log(type);
}

Configuration &IsapiServer::configuration() const
{
  return server_->configuration();
}

}

}
