/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WT_TARGET_JAVA
#include "Wt/WIOService.h"
#endif // WT_TARGET_JAVA

#include "Configuration.h"
#include "WebController.h"
#include "WebSession.h"

#include "Wt/WServer.h"
#include "Wt/Test/WTestEnvironment.h"

namespace Wt {

#ifndef WT_TARGET_JAVA
WServer::WServer(const std::string& wtApplicationPath,
		 const std::string& wtConfigurationFile)
{
  init(wtApplicationPath, wtConfigurationFile);

#ifdef WT_THREADED
  ioService().start();
#endif // WT_THREADED

  webController_ = new WebController(*this);
}

WServer::~WServer()
{
  destroy();
}
#endif // WT_TARGET_JAVA

// Not implemented: start(), stop(), isRunning(), resume(), httpPort()

  namespace Test {

#ifndef WT_TARGET_JAVA

WTestEnvironment::WTestEnvironment(EntryPointType type)
{
  server_ = new WServer(std::string(), std::string());
  if (!server_->controller())
    throw WException("Error: WTestEnvironment() could not instantiate WServer,"
		     " make sure to link only against libwttest and no other"
		     " connector library.");

  controller_ = server_->controller();

  init(type);
}

WTestEnvironment::WTestEnvironment(const std::string& applicationPath,
				   const std::string& configurationFile,
				   EntryPointType type)
{
  server_ = new WServer(applicationPath, configurationFile);
  controller_ = server_->controller();

  init(type);
}

#else

class TestController : public WebController {
public:
  TestController(Configuration *configuration);
};

WTestEnvironment::WTestEnvironment(Configuration *configuration,
				   EntryPointType type)
{
  std::vector<std::string> dummy;

  controller_ = new TestController(configuration);

  init(type);
}
#endif

void WTestEnvironment::init(EntryPointType type)
{
  session_ = new WebSession(controller_, "testwtd", type, "", 0, this);
  theSession_.reset(session_);

#ifndef WT_TARGET_JAVA
  controller_->addSession(theSession_);
#endif // WT_TARGET_JAVA

  new WebSession::Handler(theSession_, WebSession::Handler::LockOption::TakeLock);

  doesAjax_ = true;
  doesCookies_ = true;
  dpiScale_ = 1;

  urlScheme_ = "http";
  referer_ = "";
  accept_ = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
  serverSignature_ = "None (WTestEnvironment)";
  serverSoftware_ = serverSignature_;
  serverAdmin_ = "your@onyourown.here";
  pathInfo_ = "";

  setUserAgent("Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.0.11) "
	       "Gecko/2009060309 Ubuntu/9.04 (jaunty) Firefox/3.0.11");

  host_ = "localhost";
  clientAddress_ = "127.0.0.1";
  locale_ = WLocale("en");
}

#ifdef WT_TARGET_JAVA
void WTestEnvironment::close()
{
  WebSession::Handler::instance()->release();
}
#endif // WTextEnvironment

void WTestEnvironment::endRequest()
{
  delete WebSession::Handler::instance();
}

void WTestEnvironment::startRequest()
{
  new WebSession::Handler(theSession_, WebSession::Handler::LockOption::TakeLock);
}

WTestEnvironment::~WTestEnvironment()
{
  endRequest();

#ifndef WT_TARGET_JAVA
  controller_->removeSession(theSession_->sessionId());
#endif // WT_TARGET_JAVA

  theSession_.reset();

#ifndef WT_TARGET_JAVA
  delete server_;
#endif // WT_TARGET_JAVA
}

void WTestEnvironment::setParameterMap(const Http::ParameterMap& parameters)
{
  parameters_ = parameters;
}

void WTestEnvironment::setCookies(const CookieMap& cookies)
{
  cookies_ = cookies;
}

void WTestEnvironment::setHeaderValue(const std::string& value)
{
  // FIXME
}

void WTestEnvironment::setSupportsCookies(bool enabled)
{
  doesCookies_ = enabled;
}

void WTestEnvironment::setAjax(bool enabled)
{
  doesAjax_ = enabled;
}

void WTestEnvironment::setDpiScale(double dpiScale)
{
  dpiScale_ = dpiScale;
}

void WTestEnvironment::setLocale(const WLocale& locale)
{
  locale_ = locale;
}

void WTestEnvironment::setHostName(const std::string& hostName)
{
  host_ = hostName;
}

void WTestEnvironment::setUrlScheme(const std::string& scheme)
{
  urlScheme_ = scheme;
}

void WTestEnvironment::setReferer(const std::string& referer)
{
  referer_ = referer;
}

void WTestEnvironment::setAccept(const std::string& accept)
{
  accept_ = accept;
}

void WTestEnvironment::setServerSignature(const std::string& signature)
{
  serverSignature_ = signature;
}

void WTestEnvironment::setServerSoftware(const std::string& software)
{
  serverSignature_ = software;
}

void WTestEnvironment::setServerAdmin(const std::string& serverAdmin)
{
  serverAdmin_ = serverAdmin;
}

void WTestEnvironment::setClientAddress(const std::string& clientAddress)
{
  clientAddress_ = clientAddress;
}

void WTestEnvironment::setInternalPath(const std::string& internalPath)
{
  WEnvironment::setInternalPath(internalPath);
}

#ifndef WT_TARGET_JAVA
void WTestEnvironment::setAppRoot(const std::string &appRoot)
{
  server_->setAppRoot(appRoot);
}

void WTestEnvironment::setDocRoot(const std::string &docRoot)
{
  session_->setDocRoot(docRoot);
}
#endif // WT_TARGET_JAVA

void WTestEnvironment::setUserAgent(const std::string& userAgent)
{
  WEnvironment::setUserAgent(userAgent);
}

bool WTestEnvironment::isTest() const
{
  return true;
}

void WTestEnvironment::setSessionIdInUrl(bool sessionIdInUrl)
{
  theSession_->setSessionIdInUrl(sessionIdInUrl);
}

Signal<WDialog *>& WTestEnvironment::dialogExecuted() const
{
  return dialogExecuted_;
}

Signal<WPopupMenu *>& WTestEnvironment::popupExecuted() const
{
  return popupExecuted_;
}

  }
}
