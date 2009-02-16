/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

//#define NOSPIRIT

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLogger"

#include "CgiParser.h"
#include "WebRequest.h"
#include "WebSession.h"
#include "WebController.h"
#include "Configuration.h"
#include "Utils.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#ifndef NOSPIRIT
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>
#include <boost/bind.hpp>
#endif // NOSPIRIT
#include <assert.h>

namespace Wt {

WEnvironment::WEnvironment(WebSession *session)
  : session_(session),
    doesJavaScript_(false),
    doesAjax_(false),
    doesCookies_(false),
    dpiScale_(1),
    contentType_(HTML4),
    request_(0)
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
    ArgumentValues v;
    v.push_back(internalPath_);
    arguments_["historyKey"] = v;
  }
}

void WEnvironment::init(const CgiParser& cgi, const WebRequest& request)
{
  for (CgiParser::EntryMap::const_iterator i = cgi.entries().begin();
       i != cgi.entries().end(); ++i) {
    ArgumentValues v;

    CgiEntry *e = i->second;

    while (e) {
      v.push_back(e->value());
      e = e->next();
    }

    arguments_[i->first] = v;
  }

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

  isSpiderBot_ = false;

  static const char *bots[] =
    { "Googlebot", "msnbot", "Slurp", "Crawler", "Bot", "ia_archiver" };

  for (int i = 0; i < 6; ++i)
    if (userAgent_.find(bots[i]) != std::string::npos) {
      isSpiderBot_ = true;
      break;
    }

  /*
   * Determine server host name
   */
  if (WebController::conf().behindReverseProxy()) {
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

#ifndef NOSPIRIT
  if (doesCookies_)
    parseCookies(cookie);

  locale_ = parsePreferredAcceptValue(request.headerValue("Accept-Language"));
#endif // NOSPIRIT

  if (WebController::conf().sendXHTMLMimeType()
      && (accept_.find("application/xhtml+xml") != std::string::npos))
    contentType_ = XHTML1;
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

const WEnvironment::ArgumentValues&
WEnvironment::getArgument(const std::string& argument_name) const
{
  ArgumentMap::const_iterator i = arguments_.find(argument_name);

  if (i == arguments_.end())
    throw std::runtime_error("missing argument: " + argument_name);
  else
    return i->second;
}

const std::string WEnvironment::getCookie(const std::string& cookie_name) const
{
  CookieMap::const_iterator i = cookies_.find(cookie_name);

  if (i == cookies_.end())
    throw std::runtime_error("missing cookie: " + cookie_name);
  else
    return i->second;
}

std::string WEnvironment::getCgiValue(const std::string& varName) const
{
  if (request_)
    return request_->envValue(varName);
  else
    return "";
}

void WEnvironment::setRequest(WebRequest *request)
{
  request_ = request;
}

#ifndef NOSPIRIT
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

namespace {
  using namespace boost::spirit;
  using namespace boost;

  /*
   * My second spirit parser -- spirit is still nifty !
   *
   * Parses Cookies set by the browser, as defined in RFC 2965
   */
  class CookieParser : public grammar<CookieParser>
  {
    struct ParseState
    {
      std::string lastName;
    };

  public:
    CookieParser(std::map<std::string, std::string>& cookies)
      : cookies_(cookies),
	constParseState_(ParseState()),
	parseState_(constParseState_)
    { }

  private:
    std::map<std::string, std::string>& cookies_;
    ParseState  constParseState_;
    ParseState& parseState_;

    void setName(char const* str, char const* end) const {
      parseState_.lastName = std::string(str, end);
      cookies_[parseState_.lastName] = "";
    }

    void setValue(char const* str, char const* end) const {
      cookies_[parseState_.lastName] = std::string(str, end);
    }

    static std::string unescape(std::string value) {
      std::string::size_type pos;

      while ((pos = value.find('\\')) != std::string::npos) {
	value.erase(pos, 1);
      }

      return value;
    }

    void setQuotedValue(char const* str, char const* end) const {
      cookies_[parseState_.lastName] = unescape(std::string(str + 1, end - 1));
    }

    typedef CookieParser self_t;

  public:
    template <typename ScannerT>
    struct definition
    {
      definition(CookieParser const& self)
      {
	token 
	  = lexeme_d[+(alnum_p | ch_p('-') | chset_p("_.*$#|()"))]
	  ;

	quoted_string
	  = lexeme_d[ch_p('"')
		     >> *(('\\' >> anychar_p) | ~ch_p('"')) >> ch_p('"')]
	  ;

	name
	  = token
	    [
	      bind(&self_t::setName, self, _1, _2)
	    ]
	  ;

	value
	  = token
	    [
	      bind(&self_t::setValue, self, _1, _2)
	    ]
	  | quoted_string
	    [
	      bind(&self_t::setQuotedValue, self, _1, _2)
	    ]
	  ;

	cookie
	  = name >> !(ch_p('=') >> value)
	  ;

	cookielist
	  = !(cookie >> *((ch_p(',') | ch_p(';')) >> cookie)) >> end_p
	  ;
      }

      rule<ScannerT> token, quoted_string, name, value, cookie, cookielist;

      rule<ScannerT> const&
      start() const { return cookielist; }
    };
  };
};

void WEnvironment::parseCookies(const std::string& str)
{
  CookieParser cookieParser(cookies_);

  using namespace boost::spirit;

  parse_info<> info = parse(str.c_str(), cookieParser, space_p);

  if (!info.full)
    // wApp is not yet initialized here
    std::cerr << "Could not parse 'Cookie: "
	      << str << "', stopped at: '" << info.stop 
	      << '\'' << std::endl;
}

#endif // NOSPIRIT

}
