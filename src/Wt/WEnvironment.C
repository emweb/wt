/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WEnvironment"

#include "Wt/Utils"
#include "Wt/WException"
#include "Wt/WLogger"
#include "Wt/WSslInfo"
#include "Wt/Http/Request"
#include "Wt/Json/Parser"
#include "Wt/Json/Object"
#include "Wt/Json/Array"
#include "Wt/WString"

#include "WebController.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebUtils.h"
#include "Configuration.h"

#include <boost/lexical_cast.hpp>

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
  : session_(0),
    doesAjax_(false),
    doesCookies_(false),
    internalPathUsingFragments_(false),
    screenWidth_(-1),
    screenHeight_(-1),
    dpiScale_(1),
    webGLsupported_(false),
    timeZoneOffset_(0)
#ifndef WT_TARGET_JAVA
    , sslInfo_(0)
#endif
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
#ifndef WT_TARGET_JAVA
    , sslInfo_(0)
#endif
{ }

WEnvironment::~WEnvironment()
{
#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
  delete sslInfo_;
#endif
#endif
}

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

  if(conf.behindReverseProxy()) {
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
#ifndef WT_TARGET_JAVA
  if (conf.behindReverseProxy() || server()->dedicatedSessionProcess()) {
#else
  if (conf.behindReverseProxy()){
#endif
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

  sslInfo_         = request.sslInfo();
  if(!sslInfo_ && !str(request.headerValue("SSL-Client-Certificates")).empty()) {
	parseSSLInfo(str(request.headerValue("SSL-Client-Certificates")));
  }
#endif

  setUserAgent(str(request.headerValue("User-Agent")));
  updateUrlScheme(request);

  LOG_INFO("UserAgent: " << userAgent_);

  /*
   * If behind a reverse proxy, use external host, schema as communicated using 'X-Forwarded'
   * headers.
   */
#ifndef WT_TARGET_JAVA
  if (conf.behindReverseProxy() || server()->dedicatedSessionProcess()) {
#else
	if (conf.behindReverseProxy()){
#endif
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

  clientAddress_ = getClientAddress(request, conf);

  const char *cookie = request.headerValue("Cookie");
  doesCookies_ = cookie;

  if (cookie)
    parseCookies(cookie, cookies_);

  locale_ = request.parseLocale();
}

#ifndef WT_TARGET_JAVA
void WEnvironment::parseSSLInfo(const std::string& json) {
#ifdef WT_WITH_SSL
	Wt::Json::Object obj;
	Wt::Json::ParseError error;
	if(!Wt::Json::parse(Wt::Utils::base64Decode(json), obj, error)) {
	  LOG_ERROR("error while parsing client certificates");
	  return;
	}

	std::string clientCertificatePem = obj["client-certificate"];
  
	X509* cert = Wt::Ssl::readFromPem(clientCertificatePem);

	if(cert) {
	  Wt::WSslCertificate clientCert = Wt::Ssl::x509ToWSslCertificate(cert);
	  X509_free(cert);

	  Wt::Json::Array arr = obj["client-pem-certification-chain"];

	  std::vector<Wt::WSslCertificate> clientCertChain;

	  for(unsigned int i = 0; i < arr.size(); ++i ) {
		clientCertChain.push_back(Wt::Ssl::x509ToWSslCertificate(Wt::Ssl::readFromPem(arr[i])));
	  }

	  Wt::WValidator::State state = static_cast<Wt::WValidator::State>((int)obj["client-verification-result-state"]);
	  Wt::WString message = obj["client-verification-result-message"];

	  sslInfo_ = new Wt::WSslInfo(clientCert, clientCertChain, Wt::WValidator::Result(state, message));
	}
#endif // WT_WITH_SSL
}
#endif // WT_TARGET_JAVA

std::string WEnvironment::getClientAddress(const WebRequest& request,
					   const Configuration& conf)
{
  std::string result;

  /*
   * Determine client address, taking into account proxies
   */
  if (conf.behindReverseProxy()) {
    std::string clientIp = str(request.headerValue("Client-IP"));
    boost::trim(clientIp);

    std::vector<std::string> ips;
    if (!clientIp.empty())
      boost::split(ips, clientIp, boost::is_any_of(","));

    std::string forwardedFor = str(request.headerValue("X-Forwarded-For"));
    boost::trim(forwardedFor);

    std::vector<std::string> forwardedIps;
    if (!forwardedFor.empty())
      boost::split(forwardedIps, forwardedFor, boost::is_any_of(","));

    Utils::insert(ips, forwardedIps);

    for (unsigned i = 0; i < ips.size(); ++i) {
      result = ips[i];

      boost::trim(result);

      if (!result.empty()
	  && !boost::starts_with(result, "10.")
	  && !boost::starts_with(result, "172.16.")
	  && !boost::starts_with(result, "192.168.")) {
	break;
      }
    }
  }

  if (result.empty())
    result = str(request.envValue("REMOTE_ADDR"));

  return result;
}

void WEnvironment::enableAjax(const WebRequest& request)
{
  doesAjax_ = true;
  session_->controller()->newAjaxSession();

  doesCookies_ = request.headerValue("Cookie") != 0;

  if (!request.getParameter("htmlHistory"))
    internalPathUsingFragments_ = true;

  const std::string *scaleE = request.getParameter("scale");

  try {
    dpiScale_ = scaleE ? boost::lexical_cast<double>(*scaleE) : 1;
  } catch (boost::bad_lexical_cast &e) {
    dpiScale_ = 1;
  }

  const std::string *webGLE = request.getParameter("webGL");

  webGLsupported_ = webGLE ? (*webGLE == "true") : false;

  const std::string *tzE = request.getParameter("tz");

  try {
    timeZoneOffset_ = tzE ? boost::lexical_cast<int>(*tzE) : 0;
  } catch (boost::bad_lexical_cast &e) {
  }

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
      screenWidth_ = boost::lexical_cast<int>(*scrWE);
    } catch (boost::bad_lexical_cast &e) {
    }
  }
  const std::string *scrHE = request.getParameter("scrH");
  if (scrHE) {
    try {
      screenHeight_ = boost::lexical_cast<int>(*scrHE);
    } catch (boost::bad_lexical_cast &e) {
    }
  }
}

void WEnvironment::setUserAgent(const std::string& userAgent)
{
  userAgent_ = userAgent;

  Configuration& conf = session_->controller()->configuration();

  agent_ = Unknown;

  /* detecting MSIE is as messy as their browser */
  if (userAgent_.find("Trident/4.0") != std::string::npos) {
    agent_ = IE8; return;
  } if (userAgent_.find("Trident/5.0") != std::string::npos) {
    agent_ = IE9; return;
  } else if (userAgent_.find("Trident/6.0") != std::string::npos) {
    agent_ = IE10; return;
  } else if (userAgent_.find("Trident/") != std::string::npos) {
    agent_ = IE11; return;
  } else if (userAgent_.find("MSIE 2.") != std::string::npos
      || userAgent_.find("MSIE 3.") != std::string::npos
      || userAgent_.find("MSIE 4.") != std::string::npos
      || userAgent_.find("MSIE 5.") != std::string::npos
      || userAgent_.find("IEMobile") != std::string::npos)
    agent_ = IEMobile;
  else if (userAgent_.find("MSIE 6.") != std::string::npos)
    agent_ = IE6;
  else if (userAgent_.find("MSIE 7.") != std::string::npos)
    agent_ = IE7;
  else if (userAgent_.find("MSIE 8.") != std::string::npos)
    agent_ = IE8;
  else if (userAgent_.find("MSIE 9.") != std::string::npos)
    agent_ = IE9;
  else if (userAgent_.find("MSIE") != std::string::npos)
    agent_ = IE10;

  if (userAgent_.find("Opera") != std::string::npos) {
    agent_ = Opera;

    std::size_t t = userAgent_.find("Version/");
    if (t != std::string::npos) {
      std::string vs = userAgent_.substr(t + 8);
      t = vs.find(' ');
      if (t != std::string::npos)
	vs = vs.substr(0, t);
      try {
	double v = boost::lexical_cast<double>(vs);
	if (v >= 10)
	  agent_ = Opera10;
      } catch (boost::bad_lexical_cast& e) { }
    }
  }

  if (userAgent_.find("Chrome") != std::string::npos) {
    if (userAgent_.find("Android") != std::string::npos)
      agent_ = MobileWebKitAndroid;
    else if (userAgent_.find("Chrome/0.") != std::string::npos)
      agent_ = Chrome0;
    else if (userAgent_.find("Chrome/1.") != std::string::npos)
      agent_ = Chrome1;
    else if (userAgent_.find("Chrome/2.") != std::string::npos)
      agent_ = Chrome2;
    else if (userAgent_.find("Chrome/3.") != std::string::npos)
      agent_ = Chrome3;
    else if (userAgent_.find("Chrome/4.") != std::string::npos)
      agent_ = Chrome4;
    else
      agent_ = Chrome5;
  } else if (userAgent_.find("Safari") != std::string::npos) {
    if (userAgent_.find("iPhone") != std::string::npos
	|| userAgent_.find("iPad") != std::string::npos) {
      agent_ = MobileWebKitiPhone;
    } else if (userAgent_.find("Android") != std::string::npos) {
      agent_ = MobileWebKitAndroid;
    } else if (userAgent_.find("Mobile") != std::string::npos) {
      agent_ = MobileWebKit;
    } else if (userAgent_.find("Version") == std::string::npos) {
      if (userAgent_.find("Arora") != std::string::npos)
	agent_ = Arora;
      else
	agent_ = Safari;
    } else if (userAgent_.find("Version/3") != std::string::npos)
      agent_ = Safari3;
    else
      agent_ = Safari4;
  } else if (userAgent_.find("WebKit") != std::string::npos) {
    if (userAgent_.find("iPhone") != std::string::npos)
      agent_ = MobileWebKitiPhone;
    else
      agent_ = WebKit;
  } else if (userAgent_.find("Konqueror") != std::string::npos)
    agent_ = Konqueror;
  else if (userAgent_.find("Gecko") != std::string::npos)
    agent_ = Gecko;

  if (userAgent_.find("Firefox") != std::string::npos) {
    if (userAgent_.find("Firefox/0.") != std::string::npos)
      agent_ = Firefox;
    else if (userAgent_.find("Firefox/1.") != std::string::npos)
      agent_ = Firefox;
    else if (userAgent_.find("Firefox/2.") != std::string::npos)
      agent_ = Firefox;
    else {
      if (userAgent_.find("Firefox/3.0") != std::string::npos)
	agent_ = Firefox3_0;
      else if (userAgent_.find("Firefox/3.1") != std::string::npos)
	agent_ = Firefox3_1;
      else if (userAgent_.find("Firefox/3.1b") != std::string::npos)
	agent_ = Firefox3_1b;
      else if (userAgent_.find("Firefox/3.5") != std::string::npos)
	agent_ = Firefox3_5;
      else if (userAgent_.find("Firefox/3.6") != std::string::npos)
	agent_ = Firefox3_6;
      else if (userAgent_.find("Firefox/4.") != std::string::npos)
	agent_ = Firefox4_0;
      else
	agent_ = Firefox5_0;
    }
  }

  if (userAgent_.find("Edge/") != std::string::npos) {
    agent_ = Edge;
  }

  if (conf.agentIsBot(userAgent_))
    agent_ = BotAgent;
}

bool WEnvironment::agentSupportsAjax() const
{
  Configuration& conf = session_->controller()->configuration();

  return conf.agentSupportsAjax(userAgent_);
}

bool WEnvironment::supportsCss3Animations() const
{
  return ((agentIsGecko() && agent_ >= Firefox5_0) ||
	  (agentIsIE() && agent_ >= IE10) ||
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

std::string WEnvironment::sessionId() const
{
  return session_->sessionId();
}

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
    return 0;
}

const std::string WEnvironment::getCookie(const std::string& cookieName) const
{
  CookieMap::const_iterator i = cookies_.find(cookieName);

  if (i == cookies_.end())
    throw std::runtime_error("Missing cookie: " + cookieName);
  else
    return i->second;
}

const std::string *WEnvironment::getCookieValue(const std::string& cookieName)
  const
{
  CookieMap::const_iterator i = cookies_.find(cookieName);

  if (i == cookies_.end())
    return 0;
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
