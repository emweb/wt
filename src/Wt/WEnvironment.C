/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLogger"
#include "Wt/WRegExp"

#include "WebRequest.h"
#include "WebSession.h"
#include "WebController.h"
#include "Configuration.h"
#include "Utils.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <assert.h>

namespace Wt {

  namespace {
    bool regexMatchAny(const std::string& agent,
		       const std::vector<std::string>& regexList) {
      WT_USTRING s = WT_USTRING::fromUTF8(agent);
      for (unsigned i = 0; i < regexList.size(); ++i) {
	WRegExp expr(WT_USTRING::fromUTF8(regexList[i]));

	if (expr.exactMatch(s))
	  return true;
      }

      return false;
    }
  }

WEnvironment::WEnvironment()
{ }

WEnvironment::WEnvironment(WebSession *session)
  : session_(session),
    doesAjax_(false),
    doesCookies_(false),
    dpiScale_(1),
    contentType_(HTML4)
{ }

void WEnvironment::setInternalPath(const std::string& path)
{
  internalPath_ = path.empty() ? "/" : path;

#ifndef WT_TARGET_JAVA
  // emulate historyKey argument for < Wt-2.2
  if (!path.empty()) {
    Http::ParameterValues v;
    v.push_back(internalPath_);
    parameters_["historyKey"] = v;
  }
#endif // WT_TARGET_JAVA
}

const std::string& WEnvironment::deploymentPath() const
{
  return session_->deploymentPath();
}

void WEnvironment::init(const WebRequest& request)
{
  Configuration& conf = session_->controller()->configuration();

  parameters_ = request.getParameterMap();

  urlScheme_       = request.urlScheme();
  referer_         = request.headerValue("Referer");
  accept_          = request.headerValue("Accept");
  serverSignature_ = request.envValue("SERVER_SIGNATURE");
  serverSoftware_  = request.envValue("SERVER_SOFTWARE");
  serverAdmin_     = request.envValue("SERVER_ADMIN");
  pathInfo_        = request.pathInfo();

  setUserAgent(request.headerValue("User-Agent"));

  std::cerr << userAgent_ << std::endl;

  /*
   * Determine server host name
   */
  if (conf.behindReverseProxy()) {
    /*
     * Take the last entry in X-Forwarded-Host, assuming that we are only
     * behind 1 proxy
     */
    std::string forwardedHost = request.headerValue("X-Forwarded-Host");

    if (!forwardedHost.empty()) {
      std::string::size_type i = forwardedHost.rfind(',');
      if (i == std::string::npos)
	host_ = forwardedHost;
      else
	host_ = forwardedHost.substr(i+1);
    } else
      host_ = request.headerValue("Host");
  } else
    host_ = request.headerValue("Host");

  if (host_.empty()) {
    /*
     * HTTP 1.0 doesn't require it: guess from config
     */
    host_ = request.serverName();
    if (!request.serverPort().empty())
      host_ += ":" + request.serverPort();
  }

  /*
   * Determine client address, taking into account proxies
   */
  std::string ips;
  ips = request.headerValue("Client-IP") + ","
    + request.headerValue("X-Forwarded-For");

  for (std::string::size_type pos = 0; pos != std::string::npos;) {
    std::string::size_type komma_pos = ips.find(',', pos);
    clientAddress_ = ips.substr(pos, komma_pos);

    boost::trim(clientAddress_);

    if (!boost::starts_with(clientAddress_, "10.")
	&& !boost::starts_with(clientAddress_, "172.16.")
	&& !boost::starts_with(clientAddress_, "192.168.")) {
      break;
    }

    if (komma_pos != std::string::npos)
      pos = komma_pos + 1;
  }

  if (clientAddress_.empty())
    clientAddress_ = request.envValue("REMOTE_ADDR");

  std::string cookie = request.headerValue("Cookie");
  doesCookies_ = !cookie.empty();

  if (doesCookies_)
    parseCookies(cookie);

  locale_ = request.parseLocale();

  /*
   * checked=\"checked\" seems not to work with IE9 XHTML mode
   */
  if (session_->controller()->configuration().sendXHTMLMimeType()
      && (accept_.find("application/xhtml+xml") != std::string::npos)
      && !agentIsIE())
    contentType_ = XHTML1;
}

void WEnvironment::setUserAgent(const std::string& userAgent)
{
  userAgent_ = userAgent;

  Configuration& conf = session_->controller()->configuration();

  agent_ = Unknown;

  if (userAgent_.find("MSIE 2") != std::string::npos
      || userAgent_.find("MSIE 3") != std::string::npos
      || userAgent_.find("MSIE 4") != std::string::npos
      || userAgent_.find("MSIE 5") != std::string::npos
      || userAgent_.find("IEMobile") != std::string::npos)
    agent_ = IEMobile;
  else if (userAgent_.find("MSIE 6") != std::string::npos)
    agent_ = IE6;
  else if (userAgent_.find("MSIE 7") != std::string::npos)
    agent_ = IE7;
  else if (userAgent_.find("MSIE 8") != std::string::npos)
    agent_ = IE8;
  else if (userAgent_.find("MSIE") != std::string::npos)
    agent_ = IE9;

  if (userAgent_.find("Opera") != std::string::npos)
    agent_ = Opera;

  if (userAgent_.find("Chrome") != std::string::npos) {
    if (userAgent_.find("Chrome/0") != std::string::npos)
      agent_ = Chrome0;
    else if (userAgent_.find("Chrome/1") != std::string::npos)
      agent_ = Chrome1;
    else if (userAgent_.find("Chrome/2") != std::string::npos)
      agent_ = Chrome2;
    else if (userAgent_.find("Chrome/3") != std::string::npos)
      agent_ = Chrome3;
    else if (userAgent_.find("Chrome/4") != std::string::npos)
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
    if (userAgent_.find("Firefox/0") != std::string::npos)
      agent_ = Firefox;
    else if (userAgent_.find("Firefox/1") != std::string::npos)
      agent_ = Firefox;
    else if (userAgent_.find("Firefox/2") != std::string::npos)
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
      else
	agent_ = Firefox3_6;
    }
  }

  if (regexMatchAny(userAgent_, conf.botList()))
    agent_ = BotAgent;
}

bool WEnvironment::agentSupportsAjax() const
{
  Configuration& conf = session_->controller()->configuration();

  bool matches = regexMatchAny(userAgent_, conf.ajaxAgentList());
  if (conf.ajaxAgentWhiteList())
    return matches;
  else
    return !matches;
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

const std::string WEnvironment::getCookie(const std::string& cookieNname) const
{
  CookieMap::const_iterator i = cookies_.find(cookieNname);

  if (i == cookies_.end())
    throw std::runtime_error("Missing cookie: " + cookieNname);
  else
    return i->second;
}

const std::string WEnvironment::headerValue(const std::string& name) const
{
  return session_->getCgiHeader(name);
}

std::string WEnvironment::getCgiValue(const std::string& varName) const
{
  return session_->getCgiValue(varName);
}

#ifndef WT_TARGET_JAVA
WAbstractServer *WEnvironment::server() const
{
  return session_->controller()->server_;
}
#endif // WT_TARGET_JAVA

void WEnvironment::parseCookies(const std::string& str)
{
  // Cookie parsing strategy:
  // - First, split the string on cookie separators (-> name-value pair).
  //   ';' is cookie separator. ',' is not a cookie separator (as in PHP)
  // - Then, split the name-value pairs on the first '='
  // - URL decoding/encoding
  // - Trim the name, trim the value
  // - If a name-value pair does not contain an '=', the name-value pair
  //   was the name of the cookie and the value is empty

  std::vector<std::string> cookies;
  boost::split(cookies, str, boost::is_any_of(";"));
  for (unsigned int i = 0; i < cookies.size(); ++i) {
    std::string::size_type e = cookies[i].find('=');
    std::string cookieName = cookies[i].substr(0, e);
    std::string cookieValue =
      (e != std::string::npos && cookies[i].size() > e + 1) ?
        cookies[i].substr(e + 1) : "";

    boost::trim(cookieName);
    boost::trim(cookieValue);

    Wt::Utils::urlDecode(cookieName);
    Wt::Utils::urlDecode(cookieValue);
    if (cookieName != "") {
      cookies_[cookieName] = cookieValue;
    }
  }
}

}
