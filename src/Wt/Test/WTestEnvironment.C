/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebController.h"
#include "Configuration.h"
#include "WebSession.h"
#include "Wt/Test/WTestEnvironment"

namespace Wt {

  namespace Test {

#ifndef WT_TARGET_JAVA
WTestEnvironment::WTestEnvironment(const std::string& applicationPath,
				   const std::string& configurationFile,
				   EntryPointType type)
{
  configuration_ = new Configuration(applicationPath, "", configurationFile,
				     Configuration::WtHttpdServer,
				     "Wt: initializing test environment");

  controller_ = new WebController(*configuration_, 0, 0);

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

  configuration_ = configuration;
  controller_ = new TestController(configuration);

  init(type);
}
#endif

void WTestEnvironment::init(EntryPointType type)
{
  session_ = new WebSession(controller_, "testwtd", type, "", 0, this);
  theSession_.reset(session_);

  new WebSession::Handler(theSession_, true);

  doesAjax_ = true;
  doesCookies_ = true;
  dpiScale_ = 1;
  contentType_ = XHTML1;

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
  locale_ = WT_LOCALE("en");
}

WTestEnvironment::~WTestEnvironment()
{
  delete WebSession::Handler::instance();
  theSession_.reset();

  delete controller_;
  delete configuration_;
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

void WTestEnvironment::setLocale(const WT_LOCALE& locale)
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

void WTestEnvironment::setContentType(ContentType contentType)
{
  contentType_ = contentType;
}

void WTestEnvironment::setUserAgent(const std::string& userAgent)
{
  WEnvironment::setUserAgent(userAgent);
}

  }
}
