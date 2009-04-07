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
#ifndef WT_NO_SPIRIT
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>
#include <boost/bind.hpp>
#endif // WT_NO_SPIRIT
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

WEnvironment::WEnvironment(WebSession *session)
  : session_(session),
    doesJavaScript_(false),
    doesAjax_(false),
    doesCookies_(false),
    dpiScale_(1),
    contentType_(HTML4)
{ }

#if WIN32
// Because class is WT_API, all methods must be defined
WEnvironment::WEnvironment(const WEnvironment &)
{
  assert(false);
}
#endif

void WEnvironment::setInternalPath(const std::string& path)
{
  internalPath_ = path.empty() ? "/" : path;

  // emulate historyKey argument for < Wt-2.2
  if (!path.empty()) {
    Http::ParameterValues v;
    v.push_back(internalPath_);
    parameters_["historyKey"] = v;
  }
}

void WEnvironment::init(const WebRequest& request)
{
  Configuration& conf = session_->controller()->configuration();

  parameters_ = request.getParameterMap();

  urlScheme_       = request.urlScheme();
  userAgent_       = request.headerValue("User-Agent");
  referer_         = request.headerValue("Referer");
  accept_          = request.headerValue("Accept");
  serverSignature_ = request.envValue("SERVER_SIGNATURE");
  serverSoftware_  = request.envValue("SERVER_SOFTWARE");
  serverAdmin_     = request.envValue("SERVER_ADMIN");
  pathInfo_        = request.pathInfo();

  isIEMobile_ =
       userAgent_.find("MSIE 4") != std::string::npos
    || userAgent_.find("MSIE 5") != std::string::npos
    || userAgent_.find("IEMobile") != std::string::npos;

  isIE_ = userAgent_.find("MSIE") != std::string::npos;
  isIE6_ = !isIEMobile_ && userAgent_.find("MSIE 6") != std::string::npos;
  isOpera_ = userAgent_.find("Opera") != std::string::npos;
  isSafari_ = userAgent_.find("Safari") != std::string::npos;
  isWebKit_ = userAgent_.find("WebKit") != std::string::npos;
  isKonqueror_ = userAgent_.find("Konqueror") != std::string::npos;
  isGecko_ = !isSafari_ && userAgent_.find("Gecko") != std::string::npos;
  isSpiderBot_ = regexMatchAny(userAgent_, conf.botList());

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

  locale_ = parsePreferredAcceptValue(request.headerValue("Accept-Language"));

  if (session_->controller()->configuration().sendXHTMLMimeType()
      && (accept_.find("application/xhtml+xml") != std::string::npos))
    contentType_ = XHTML1;
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

void WEnvironment::libraryVersion(int& series, int& major, int& minor) const
{
  series = WT_SERIES;
  major = WT_MAJOR;
  minor = WT_MINOR;
}

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
  if (!values.empty())
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

std::string WEnvironment::getCgiValue(const std::string& varName) const
{
  return session_->getCgiValue(varName);
}

#ifndef WT_NO_SPIRIT
namespace {
  using namespace boost::spirit;
  using namespace boost;

  /*
   * My first spirit parser -- spirit is nifty !
   *
   * Parses things like:
   *  nl-be,en-us;q=0.7,en;q=0.3
   *  ISO-8859-1,utf-8;q=0.7,*;q=0.7
   *
   * And store the values with indicated qualities.
   */
  class ValueListParser : public grammar<ValueListParser>
  {
  public:
    struct Value {
      std::string value;
      double quality;

      Value(std::string v, double q) : value(v), quality(q) { }
    };

    ValueListParser(std::vector<Value>& values)
      : values_(values)
    { }

  private:
    std::vector<Value>& values_;

    void setQuality(double v) const {
      values_.back().quality = v;
    }

    void addValue(char const* str, char const* end) const {
      values_.push_back(Value(std::string(str, end), 1.));
    }

    typedef ValueListParser self_t;

  public:
    template <typename ScannerT>
    struct definition
    {
      definition(ValueListParser const& self)
      {
	option 
	  = ((ch_p('q') | ch_p('Q'))
	     >> '=' 
	     >> ureal_p
	        [
		  bind(&self_t::setQuality, self, _1)
		]
	     )
	  | (+alpha_p >> '=' >> +alnum_p)
	  ;

	value
	  = lexeme_d[(alpha_p >> +(alpha_p | '-')) | '*']
	    [
	       bind(&self_t::addValue, self, _1, _2)
	    ]
	    >> !( ';' >> option )
	  ;

	valuelist
	  = !(value  >> *(',' >> value )) >> end_p
	  ;
      }

      rule<ScannerT> option, value, valuelist;

      rule<ScannerT> const&
      start() const { return valuelist; }
    };
  };
};

std::string WEnvironment::parsePreferredAcceptValue(const std::string& str)
{
  std::vector<ValueListParser::Value> values;

  ValueListParser valueListParser(values);

  using namespace boost::spirit;

  parse_info<> info = parse(str.c_str(), valueListParser, space_p);

  if (info.full) {
    unsigned best = 0;
    for (unsigned i = 1; i < values.size(); ++i) {
      if (values[i].quality > values[best].quality)
	best = i;
    }

    if (best < values.size())
      return values[best].value;
    else
      return "";
  } else {
    // wApp is not yet initialized here
    std::cerr << "Could not parse 'Accept-Language: "
	      << str << "', stopped at: '" << info.stop 
	      << '\'' << std::endl;
    return "";
  }
}

#else
std::string WEnvironment::parsePreferredAcceptValue(const std::string& str)
{
  return std::string();
}
#endif // WT_NO_SPIRIT

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
