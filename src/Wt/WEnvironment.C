/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WEnvironment.h"

#include "Wt/Utils.h"
#include "Wt/WException.h"
#include "Wt/WLogger.h"
#include "Wt/WSslInfo.h"
#include "Wt/Http/Request.h"
#include "Wt/Json/Parser.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Array.h"
#include "Wt/WString.h"

#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebUtils.h"
#include "Configuration.h"

#include <boost/algorithm/string.hpp>

#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
#include <openssl/ssl.h>
#include "SslUtils.h"
#endif //WT_TARGET_JAVA
#endif //WT_WITH_SSL

namespace {
  inline std::string str(const char *s) {
    return s ? std::string(s) : std::string();
  }
}

namespace Wt {

LOGGER("WEnvironment");

WEnvironment::WEnvironment()
  : session_(nullptr),
    doesAjax_(false),
    doesCookies_(false),
    internalPathUsingFragments_(false),
    screenWidth_(-1),
    screenHeight_(-1),
    dpiScale_(1),
    webGLsupported_(false),
    timeZoneOffset_(0)
{ }

WEnvironment::WEnvironment(WebSession *session)
  : session_(session),
    doesAjax_(false),
    doesCookies_(false),
    internalPathUsingFragments_(false),
    screenWidth_(-1),
    screenHeight_(-1),
    dpiScale_(1),
    webGLsupported_(false),
    timeZoneOffset_(0)
{ }

WEnvironment::~WEnvironment()
{ }

void WEnvironment::setInternalPath(const std::string& path)
{
  if (path.empty())
    internalPath_ = path;
  else
    internalPath_ = Utils::prepend(path, '/');
}

const std::string& WEnvironment::deploymentPath() const
{
  if (!publicDeploymentPath_.empty())
    return publicDeploymentPath_;
  else
    return session_->deploymentPath();
}

void WEnvironment::updateHostName(const WebRequest& request)
{
  Configuration& conf = session_->controller()->configuration();
  std::string oldHost = host_;
  host_ = str(request.headerValue("Host"));

  if (conf.behindReverseProxy() ||
      conf.isTrustedProxy(request.remoteAddr())) {
	std::string forwardedHost = str(request.headerValue("X-Forwarded-Host"));

	if (!forwardedHost.empty()) {
	  std::string::size_type i = forwardedHost.rfind(',');
	  if (i == std::string::npos)
		host_ = forwardedHost;
	  else
		host_ = forwardedHost.substr(i+1);
	}

  }
  if(host_.size() == 0) host_ = oldHost;
}

void WEnvironment::updateUrlScheme(const WebRequest& request) 
{
  urlScheme_       = str(request.urlScheme());

  Configuration& conf = session_->controller()->configuration();

  if (conf.behindReverseProxy() ||
      conf.isTrustedProxy(request.remoteAddr())) {
    std::string forwardedProto = str(request.headerValue("X-Forwarded-Proto"));
    if (!forwardedProto.empty()) {
      std::string::size_type i = forwardedProto.rfind(',');
      if (i == std::string::npos)
        urlScheme_ = forwardedProto;
      else
        urlScheme_ = forwardedProto.substr(i+1);
    }
  }
}


void WEnvironment::init(const WebRequest& request)
{
  Configuration& conf = session_->controller()->configuration();

  queryString_ = request.queryString();
  parameters_ = request.getParameterMap();
  host_            = str(request.headerValue("Host"));
  referer_         = str(request.headerValue("Referer"));
  accept_          = str(request.headerValue("Accept"));
  serverSignature_ = str(request.envValue("SERVER_SIGNATURE"));
  serverSoftware_  = str(request.envValue("SERVER_SOFTWARE"));
  serverAdmin_     = str(request.envValue("SERVER_ADMIN"));
  pathInfo_        = request.pathInfo();

#ifndef WT_TARGET_JAVA
  if(!str(request.headerValue("Redirect-Secret")).empty())
	session_->controller()->redirectSecret_ = str(request.headerValue("Redirect-Secret"));

  sslInfo_ = request.sslInfo(conf);
#endif

  setUserAgent(str(request.headerValue("User-Agent")));
  updateUrlScheme(request);

  LOG_INFO("UserAgent: " << userAgent_);

  /*
   * If behind a reverse proxy, use external host, schema as communicated using 'X-Forwarded'
   * headers.
   */
  if (conf.behindReverseProxy() ||
      conf.isTrustedProxy(request.remoteAddr())) {
    std::string forwardedHost = str(request.headerValue("X-Forwarded-Host"));

    if (!forwardedHost.empty()) {
      std::string::size_type i = forwardedHost.rfind(',');
      if (i == std::string::npos)
	host_ = forwardedHost;
      else
	host_ = forwardedHost.substr(i+1);
    }
  }



  if (host_.empty()) {
    /*
     * HTTP 1.0 doesn't require it: guess from config
     */
    host_ = request.serverName();
    if (!request.serverPort().empty())
      host_ += ":" + request.serverPort();
  }

  clientAddress_ = request.clientAddress(conf);

  const char *cookie = request.headerValue("Cookie");
  doesCookies_ = cookie;

  if (cookie)
    parseCookies(cookie, cookies_);

  locale_ = request.parseLocale();
}


void WEnvironment::enableAjax(const WebRequest& request)
{
  doesAjax_ = true;
  session_->controller()->newAjaxSession();

  doesCookies_ = request.headerValue("Cookie") != nullptr;

  if (!request.getParameter("htmlHistory"))
    internalPathUsingFragments_ = true;

  const std::string *scaleE = request.getParameter("scale");

  try {
    dpiScale_ = scaleE ? Utils::stod(*scaleE) : 1;
  } catch (std::exception& e) {
    dpiScale_ = 1;
  }

  const std::string *webGLE = request.getParameter("webGL");

  webGLsupported_ = webGLE ? (*webGLE == "true") : false;

  const std::string *tzE = request.getParameter("tz");

  try {
    timeZoneOffset_ = std::chrono::minutes(tzE ? Utils::stoi(*tzE) : 0);
  } catch (std::exception& e) {
  }

  const std::string *tzSE = request.getParameter("tzS");

  timeZoneName_ = tzSE ? *tzSE : std::string("");

  const std::string *hashE = request.getParameter("_");

  // the internal path, when present as an anchor (#), is only
  // conveyed in the second request
  if (hashE)
    setInternalPath(*hashE);

  const std::string *deployPathE = request.getParameter("deployPath");
  if (deployPathE) {
    publicDeploymentPath_ = *deployPathE;
    std::size_t s = publicDeploymentPath_.find('/');
    if (s != 0)
      publicDeploymentPath_.clear(); // looks invalid
  }

  const std::string *scrWE = request.getParameter("scrW");
  if (scrWE) {
    try {
      screenWidth_ = Utils::stoi(*scrWE);
    } catch (std::exception &e) {
    }
  }
  const std::string *scrHE = request.getParameter("scrH");
  if (scrHE) {
    try {
      screenHeight_ = Utils::stoi(*scrHE);
    } catch (std::exception &e) {
    }
  }
}

void WEnvironment::setUserAgent(const std::string& userAgent)
{
  userAgent_ = userAgent;

  Configuration& conf = session_->controller()->configuration();

  agent_ = UserAgent::Unknown;

  /* detecting MSIE is as messy as their browser */
  if (userAgent_.find("Trident/4.0") != std::string::npos) {
    agent_ = UserAgent::IE8; return;
  } if (userAgent_.find("Trident/5.0") != std::string::npos) {
    agent_ = UserAgent::IE9; return;
  } else if (userAgent_.find("Trident/6.0") != std::string::npos) {
    agent_ = UserAgent::IE10; return;
  } else if (userAgent_.find("Trident/") != std::string::npos) {
    agent_ = UserAgent::IE11; return;
  } else if (userAgent_.find("MSIE 2.") != std::string::npos
      || userAgent_.find("MSIE 3.") != std::string::npos
      || userAgent_.find("MSIE 4.") != std::string::npos
      || userAgent_.find("MSIE 5.") != std::string::npos
      || userAgent_.find("IEMobile") != std::string::npos)
    agent_ = UserAgent::IEMobile;
  else if (userAgent_.find("MSIE 6.") != std::string::npos)
    agent_ = UserAgent::IE6;
  else if (userAgent_.find("MSIE 7.") != std::string::npos)
    agent_ = UserAgent::IE7;
  else if (userAgent_.find("MSIE 8.") != std::string::npos)
    agent_ = UserAgent::IE8;
  else if (userAgent_.find("MSIE 9.") != std::string::npos)
    agent_ = UserAgent::IE9;
  else if (userAgent_.find("MSIE") != std::string::npos)
    agent_ = UserAgent::IE10;

  if (userAgent_.find("Opera") != std::string::npos) {
    agent_ = UserAgent::Opera;

    std::size_t t = userAgent_.find("Version/");
    if (t != std::string::npos) {
      std::string vs = userAgent_.substr(t + 8);
      t = vs.find(' ');
      if (t != std::string::npos)
	vs = vs.substr(0, t);
      try {
	double v = Utils::stod(vs);
	if (v >= 10)
	  agent_ = UserAgent::Opera10;
      } catch (std::exception& e) { }
    }
  }

  if (userAgent_.find("Chrome") != std::string::npos) {
    if (userAgent_.find("Android") != std::string::npos)
      agent_ = UserAgent::MobileWebKitAndroid;
    else if (userAgent_.find("Chrome/0.") != std::string::npos)
      agent_ = UserAgent::Chrome0;
    else if (userAgent_.find("Chrome/1.") != std::string::npos)
      agent_ = UserAgent::Chrome1;
    else if (userAgent_.find("Chrome/2.") != std::string::npos)
      agent_ = UserAgent::Chrome2;
    else if (userAgent_.find("Chrome/3.") != std::string::npos)
      agent_ = UserAgent::Chrome3;
    else if (userAgent_.find("Chrome/4.") != std::string::npos)
      agent_ = UserAgent::Chrome4;
    else
      agent_ = UserAgent::Chrome5;
  } else if (userAgent_.find("Safari") != std::string::npos) {
    if (userAgent_.find("iPhone") != std::string::npos
	|| userAgent_.find("iPad") != std::string::npos) {
      agent_ = UserAgent::MobileWebKitiPhone;
    } else if (userAgent_.find("Android") != std::string::npos) {
      agent_ = UserAgent::MobileWebKitAndroid;
    } else if (userAgent_.find("Mobile") != std::string::npos) {
      agent_ = UserAgent::MobileWebKit;
    } else if (userAgent_.find("Version") == std::string::npos) {
      if (userAgent_.find("Arora") != std::string::npos)
	agent_ = UserAgent::Arora;
      else
	agent_ = UserAgent::Safari;
    } else if (userAgent_.find("Version/3") != std::string::npos)
      agent_ = UserAgent::Safari3;
    else
      agent_ = UserAgent::Safari4;
  } else if (userAgent_.find("WebKit") != std::string::npos) {
    if (userAgent_.find("iPhone") != std::string::npos)
      agent_ = UserAgent::MobileWebKitiPhone;
    else
      agent_ = UserAgent::WebKit;
  } else if (userAgent_.find("Konqueror") != std::string::npos)
    agent_ = UserAgent::Konqueror;
  else if (userAgent_.find("Gecko") != std::string::npos)
    agent_ = UserAgent::Gecko;

  if (userAgent_.find("Firefox") != std::string::npos) {
    if (userAgent_.find("Firefox/0.") != std::string::npos)
      agent_ = UserAgent::Firefox;
    else if (userAgent_.find("Firefox/1.") != std::string::npos)
      agent_ = UserAgent::Firefox;
    else if (userAgent_.find("Firefox/2.") != std::string::npos)
      agent_ = UserAgent::Firefox;
    else {
      if (userAgent_.find("Firefox/3.0") != std::string::npos)
	agent_ = UserAgent::Firefox3_0;
      else if (userAgent_.find("Firefox/3.1") != std::string::npos)
	agent_ = UserAgent::Firefox3_1;
      else if (userAgent_.find("Firefox/3.1b") != std::string::npos)
	agent_ = UserAgent::Firefox3_1b;
      else if (userAgent_.find("Firefox/3.5") != std::string::npos)
	agent_ = UserAgent::Firefox3_5;
      else if (userAgent_.find("Firefox/3.6") != std::string::npos)
	agent_ = UserAgent::Firefox3_6;
      else if (userAgent_.find("Firefox/4.") != std::string::npos)
	agent_ = UserAgent::Firefox4_0;
      else
	agent_ = UserAgent::Firefox5_0;
    }
  }

  if (userAgent_.find("Edge/") != std::string::npos) {
    agent_ = UserAgent::Edge;
  }

  if (conf.agentIsBot(userAgent_))
    agent_ = UserAgent::BotAgent;
}

bool WEnvironment::agentSupportsAjax() const
{
  Configuration& conf = session_->controller()->configuration();

  return conf.agentSupportsAjax(userAgent_);
}

bool WEnvironment::supportsCss3Animations() const
{
  return ((agentIsGecko() && 
	   static_cast<unsigned int>(agent_) >= 
	   static_cast<unsigned int>(UserAgent::Firefox5_0)) ||
	  (agentIsIE() && 
	   static_cast<unsigned int>(agent_) >= 
	   static_cast<unsigned int>(UserAgent::IE10)) ||
	  agentIsWebKit());
}

std::string WEnvironment::libraryVersion()
{
  return WT_VERSION_STR;
}

#ifndef WT_TARGET_JAVA
void WEnvironment::libraryVersion(int& series, int& major, int& minor) const
{
  series = WT_SERIES;
  major = WT_MAJOR;
  minor = WT_MINOR;
}
#endif //WT_TARGET_JAVA

const Http::ParameterValues&
WEnvironment::getParameterValues(const std::string& name) const
{
  Http::ParameterMap::const_iterator i = parameters_.find(name);

  if (i != parameters_.end())
    return i->second;
  else
    return WebRequest::emptyValues_;
}

const std::string *WEnvironment::getParameter(const std::string& name) const
{
  const Http::ParameterValues& values = getParameterValues(name);
  if (!Utils::isEmpty(values))
    return &values[0];
  else
    return nullptr;
}

const std::string *WEnvironment::getCookie(const std::string& cookieName)
  const
{
  CookieMap::const_iterator i = cookies_.find(cookieName);

  if (i == cookies_.end())
    return nullptr;
  else
    return &i->second;
}

const std::string WEnvironment::headerValue(const std::string& name) const
{
  return session_->getCgiHeader(name);
}

std::string WEnvironment::getCgiValue(const std::string& varName) const
{
  if (varName == "QUERY_STRING")
    return queryString_;
  else
    return session_->getCgiValue(varName);
}

WServer *WEnvironment::server() const
{
#ifndef WT_TARGET_JAVA
  return session_->controller()->server();
#else
  return session_->controller();
#endif // WT_TARGET_JAVA
}

bool WEnvironment::isTest() const
{
  return false;
}

void WEnvironment::parseCookies(const std::string& cookie,
				std::map<std::string, std::string>& result)
{
  // Cookie parsing strategy:
  // - First, split the string on cookie separators (-> name-value pair).
  //   ';' is cookie separator. ',' is not a cookie separator (as in PHP)
  // - Then, split the name-value pairs on the first '='
  // - URL decoding/encoding
  // - Trim the name, trim the value
  // - If a name-value pair does not contain an '=', the name-value pair
  //   was the name of the cookie and the value is empty

  std::vector<std::string> list;
  boost::split(list, cookie, boost::is_any_of(";"));
  for (unsigned int i = 0; i < list.size(); ++i) {
    std::string::size_type e = list[i].find('=');
    if (e == std::string::npos)
      continue;
    std::string cookieName = list[i].substr(0, e);
    std::string cookieValue =
      (e != std::string::npos && list[i].size() > e + 1) ?
      list[i].substr(e + 1) : "";

    boost::trim(cookieName);
    boost::trim(cookieValue);

    cookieName = Wt::Utils::urlDecode(cookieName);
    cookieValue = Wt::Utils::urlDecode(cookieValue);
    if (cookieName != "")
      result[cookieName] = cookieValue;
  }
}
Signal<WDialog *>& WEnvironment::dialogExecuted() const
{
  throw WException("Internal error");
}

Signal<WPopupMenu *>& WEnvironment::popupExecuted() const
{
  throw WException("Internal error");
}

}
