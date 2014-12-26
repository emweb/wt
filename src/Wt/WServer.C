/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WConfig.h"

#if !defined(WT_WIN32)
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#else
#include <process.h>
#endif // !_WIN32

#include <boost/algorithm/string.hpp>

#include "Wt/WIOService"
#include "Wt/WResource"
#include "Wt/WServer"

#include "Configuration.h"
#include "WebController.h"

namespace Wt {

LOGGER("WServer");

  namespace {
    bool CatchSignals = true;
  }

WServer *WServer::instance_ = 0;

WServer::Exception::Exception(const std::string& what)
  : WException(what)
{ }

void WServer::init(const std::string& wtApplicationPath,
		   const std::string& configurationFile)
{
  application_ = wtApplicationPath;
  configurationFile_ = configurationFile; 

  ownsIOService_ = true;
  ioService_ = 0;
  webController_ = 0;
  configuration_ = 0;
  localizedStrings_ = 0;

  logger_.addField("datetime", false);
  logger_.addField("app", false);
  logger_.addField("session", false);
  logger_.addField("type", false);
  logger_.addField("message", true);

  instance_ = this;
}

void WServer::destroy()
{
  if (ownsIOService_) {
    delete ioService_;
    ioService_ = 0;
  }

  delete webController_;
  delete configuration_;
  delete localizedStrings_;

  instance_ = 0;
}

void WServer::setLocalizedStrings(WLocalizedStrings *stringResolver)
{
  delete localizedStrings_;
  localizedStrings_ = stringResolver;
}

void WServer::setIOService(WIOService& ioService)
{
  if (ioService_) {
    LOG_ERROR("setIOService(): already have an IO service");
    return;
  }

  ioService_ = &ioService;
  ownsIOService_ = false;
}

WIOService& WServer::ioService()
{
  if (!ioService_) {
    ioService_ = new WIOService();
    ioService_->setThreadCount(configuration().numThreads());
  }

  return *ioService_;
}

void WServer::setAppRoot(const std::string& path)
{
  if (configuration_)
    LOG_ERROR("setAppRoot(): too late, already configured");

  appRoot_ = path;
}

std::string WServer::appRoot() const
{
  // FIXME we should const-correct Configuration too
  return const_cast<WServer *>(this)->configuration().appRoot();
}

void WServer::setConfiguration(const std::string& file)
{
  setConfiguration(file, application_);
}

void WServer::setConfiguration(const std::string& file,
			       const std::string& application)
{
  if (configuration_)
    LOG_ERROR("setConfigurationFile(): too late, already configured");

  configurationFile_ = file;
  application_ = application;
}

WLogEntry WServer::log(const std::string& type) const
{
  WLogEntry e = logger_.entry(type);

  e << WLogger::timestamp << WLogger::sep
    << getpid() << WLogger::sep
    << /* sessionId << */ WLogger::sep
    << '[' << type << ']' << WLogger::sep;

  return e;
}

void WServer::initLogger(const std::string& logFile,
			 const std::string& logConfig)
{
  if (!logFile.empty())
    logger_.setFile(logFile);

  if (!logConfig.empty())
    logger_.configure(logConfig);

  if (!description_.empty())
    LOG_INFO("initializing " << description_);
}

Configuration& WServer::configuration()
{
  if (!configuration_) {
    if (appRoot_.empty())
      appRoot_ = Configuration::locateAppRoot();
    if (configurationFile_.empty())
      configurationFile_ = Configuration::locateConfigFile(appRoot_);

    configuration_ = new Configuration(application_, appRoot_,
				       configurationFile_, this);
  }

  return *configuration_;
}

bool WServer::readConfigurationProperty(const std::string& name,
					std::string& value) const
{
  WServer *self = const_cast<WServer *>(this);
  return self->configuration().readConfigurationProperty(name, value);
}

void WServer::post(const std::string& sessionId,
		   const boost::function<void ()>& function,
		   const boost::function<void ()>& fallbackFunction)
{
  schedule(0, sessionId, function, fallbackFunction);
}

void WServer::postAll(const boost::function<void ()>& function)
{
  std::vector<std::string> sessions = webController_->sessions();
  for (std::vector<std::string>::const_iterator i = sessions.begin();
      i != sessions.end(); ++i) {
    schedule(0, *i, function);
  }
}

void WServer::schedule(int milliSeconds,
		       const std::string& sessionId,
		       const boost::function<void ()>& function,
		       const boost::function<void ()>& fallbackFunction)
{
  ApplicationEvent event(sessionId, function, fallbackFunction);

  ioService().schedule(milliSeconds,
		       boost::bind(&WebController::handleApplicationEvent,
				   webController_, event));
}

void WServer::addEntryPoint(EntryPointType type, ApplicationCreator callback,
			    const std::string& path, const std::string& favicon)
{
  if (!path.empty() && !boost::starts_with(path, "/")) 
    throw WServer::Exception("WServer::addEntryPoint() error: "
			     "deployment path should start with \'/\'");

  configuration().addEntryPoint(EntryPoint(type, callback, path, favicon));
}

void WServer::addResource(WResource *resource, const std::string& path)
{
  if (!boost::starts_with(path, "/")) 
    throw WServer::Exception("WServer::addResource() error: "
			     "static resource path should start with \'/\'");

  std::vector<EntryPoint> entryPoints = configuration().entryPoints();
  for (unsigned i = 0; i < entryPoints.size(); ++i) {
    if (entryPoints[i].resource() &&
	entryPoints[i].resource()->internalPath() == path) {
      WString error("WServer::addResource() error: "
		    "a static resource was already deployed on path '{1}'");
      throw WServer::Exception(error.arg(path).toUTF8());
    }
  }

  resource->setInternalPath(path);

  configuration().addEntryPoint(EntryPoint(resource, path));
}

void WServer::removeEntryPoint(const std::string& path){
  configuration().removeEntryPoint(path);
}

void WServer::restart(int argc, char **argv, char **envp)
{
#ifndef WT_WIN32
  char *path = realpath(argv[0], 0);

  // Try a few times since this may fail because we have an incomplete
  // binary...
  for (int i = 0; i < 5; ++i) {
    int result = execve(path, argv, envp);
    if (result != 0)
      sleep(1);
  }

  perror("execve");
#endif
}

void WServer::setCatchSignals(bool catchSignals)
{
  CatchSignals = catchSignals;
}

#if defined(WT_WIN32) && defined(WT_THREADED)

boost::mutex     terminationMutex;
bool             terminationRequested = false;
boost::condition terminationCondition;

void WServer::terminate()
{
  boost::mutex::scoped_lock terminationLock(terminationMutex);
  terminationRequested = true;
  terminationCondition.notify_all(); // should be just 1
}

BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
  switch (ctrl_type)
  {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    {
      WServer::terminate();
      return TRUE;
    }
  default:
    return FALSE;
  }
}

#endif

int WServer::waitForShutdown(const char *restartWatchFile)
{
#if !defined(WT_WIN32)
  if (!CatchSignals) {
    for(;;)
      sleep(0x1<<16);
  }
#endif // WIN32

#ifdef WT_THREADED

#if !defined(WT_WIN32)
  sigset_t wait_mask;
  sigemptyset(&wait_mask);

  // Block the signals which interest us
  sigaddset(&wait_mask, SIGHUP);
  sigaddset(&wait_mask, SIGINT);
  sigaddset(&wait_mask, SIGQUIT);
  sigaddset(&wait_mask, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &wait_mask, 0);

  for (;;) {
    int rc, sig= -1;

    // Wait for a signal to be raised
    rc= sigwait(&wait_mask, &sig);

    // branch based on return value of sigwait(). 
    switch (rc) {
      case 0: // rc indicates one of the blocked signals was raised.

        // branch based on the signal which was raised.
        switch(sig) {
          case SIGHUP: // SIGHUP means re-read the configuration.
            if (instance())
	      instance()->configuration().rereadConfiguration();
            break;

          default: // Any other blocked signal means time to quit.
            return sig;
        }

        break;
      case EINTR:
	// rc indicates an unblocked signal was raised, so we'll go
	// around again.

        break;
      default:
	// report the error and return an obviously illegitimate signal value.
        throw WServer::Exception(std::string("sigwait() error: ")
				 + strerror(rc));
        return -1;
    }
  }

#else  // WIN32

  boost::mutex::scoped_lock terminationLock(terminationMutex);
  SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
  while (!terminationRequested)
    terminationCondition.wait(terminationLock);
  SetConsoleCtrlHandler(console_ctrl_handler, FALSE);
  return 0;

#endif // WIN32
#else
  return 0;
#endif // WT_THREADED
}

void WServer::expireSessions()
{
  webController_->expireSessions();
}

void WServer::scheduleStop()
{
#ifdef WT_THREADED
  #ifndef WT_WIN32
    kill(getpid(), SIGTERM);
  #else // WT_WIN32
    terminate();
  #endif // WT_WIN32
#else // !WT_THREADED
  if (!stopCallback_.empty())
    stopCallback_();
#endif // WT_THREADED
}

}
