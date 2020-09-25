/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException.h"
#include "Wt/WLocale.h"
#include "Wt/WLogger.h"
#include "Wt/WDateTime.h"

#include "WebRequest.h"
#include "WebUtils.h"
#include "Configuration.h"

#include <cstdlib>

#include <boost/algorithm/string.hpp>

#ifndef WT_NO_SPIRIT

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#include <boost/spirit.hpp>
#else
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_attribute.hpp>
#endif

#endif // WT_NO_SPIRIT

using std::atoi;

namespace {

inline std::string str(const char *s) {
  return s ? std::string(s) : std::string();
}

bool isPrivateIP(const std::string &s) {
  return boost::starts_with(s, "127.") ||
         boost::starts_with(s, "10.") ||
         boost::starts_with(s, "192.168.") ||
         (s.size() >= 7 &&
          boost::starts_with(s, "172.") &&
          s[6] == '.' &&
          ((s[4] == '1' &&
            s[5] >= '6' &&
            s[5] <= '9') ||
           (s[4] == '2' &&
            s[5] >= '0' &&
            s[5] <= '9') ||
           (s[4] == '3' &&
            s[5] >= '0' &&
            s[5] <= '1')));
}

}

namespace Wt {

LOGGER("WebRequest");

Http::ParameterValues WebRequest::emptyValues_;

struct WebRequest::AsyncEmulation {
  bool done;

  AsyncEmulation()
    : done(false)
  { }
};

WebRequest::WebRequest()
  : entryPoint_(0),
    async_(0),
    webSocketRequest_(false)
{
#ifndef BENCH
  start_ = std::chrono::high_resolution_clock::now();
#endif
}

WebRequest::~WebRequest()
{
  delete async_;
  log();
}

void WebRequest::log()
{
#ifndef BENCH
  if (start_.time_since_epoch().count() > 0) {
    auto end = std::chrono::high_resolution_clock::now();
    double microseconds 
      = std::chrono::duration_cast<std::chrono::microseconds>(end - start_)
      .count();
    LOG_INFO("took " << (microseconds / 1000) << " ms");

    start_ = std::chrono::high_resolution_clock::time_point();
  }
#endif
}

void WebRequest::reset()
{
#ifndef BENCH
  start_ = std::chrono::high_resolution_clock::now();
#endif

  entryPoint_ = 0;
  delete async_;
  async_ = 0;
  webSocketRequest_ = false;

  parameters_.clear();
  files_.clear();

  urlParams_.clear();
}

void WebRequest::readWebSocketMessage(const ReadCallback& callback)
{ 
  throw WException("should not get here");
}

bool WebRequest::webSocketMessagePending() const
{
  throw WException("should not get here");
}

bool WebRequest::detectDisconnect(const DisconnectCallback& callback)
{
  return false; /* Not implemented */
}

const char *WebRequest::userAgent() const
{
  return headerValue("User-Agent");
}

const char *WebRequest::referer() const
{
  return headerValue("Referer");
}

const char *WebRequest::contentType() const
{
  return envValue("CONTENT_TYPE");
}

::int64_t WebRequest::contentLength() const
{
  const char *lenstr = envValue("CONTENT_LENGTH");

  if (!lenstr || strlen(lenstr) == 0)
    return 0;
  else {
    try {
      ::int64_t len = Utils::stoll(std::string(lenstr));
      if (len < 0) {
	LOG_ERROR("Bad content-length: " << lenstr);
	throw WException("Bad content-length");
      } else {
	return len;
      }
    } catch (std::exception& e) {
      LOG_ERROR("Bad content-length: " << lenstr);
      throw WException("Bad content-length");
    }
  }
}

const std::string *WebRequest::getParameter(const std::string& name) const
{
  const Http::ParameterValues& values = getParameterValues(name);

  return !values.empty() ? &values[0] : 0;
}

const Http::ParameterValues&
WebRequest::getParameterValues(const std::string& name) const
{
  Http::ParameterMap::const_iterator i = parameters_.find(name);
  if (i != parameters_.end())
    return i->second;
  else
    return emptyValues_;
}

#ifndef WT_NO_SPIRIT
namespace {
#if BOOST_VERSION < 103600
  using namespace boost::spirit;
#else
  using namespace boost::spirit::classic;
#endif

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
		  std::bind(&self_t::setQuality, self, std::placeholders::_1)
		]
	     )
	  | (+alpha_p >> '=' >> +alnum_p)
	  ;

	value
	  = lexeme_d[(alpha_p >> +(alnum_p | '-')) | '*']
	    [
	       std::bind(&self_t::addValue, self, std::placeholders::_1, std::placeholders:: _2)
	    ]
	    >> !( ';' >> option )
	  ;

	valuelist
	  = !(value  >> *(',' >> value )) >> end_p
	  ;
      }

      rule<ScannerT> option, value, valuelist;

      rule<ScannerT> const& start() const { return valuelist; }
    };
  };
}

std::string WebRequest::parsePreferredAcceptValue(const char *str) const
{
  if (!str)
    return std::string();

  std::vector<ValueListParser::Value> values;

  ValueListParser valueListParser(values);

  parse_info<> info = parse(str, valueListParser, space_p);

  if (info.full) {
    unsigned best = 0;
    for (unsigned i = 1; i < values.size(); ++i) {
      if (values[i].quality > values[best].quality)
	best = i;
    }

    if (best < values.size())
      return values[best].value;
    else
      return std::string();
  } else {
    LOG_ERROR("Could not parse 'Accept-Language: " << str
	      << "', stopped at: '" << info.stop << '\'');
    return std::string();
  }
}
#else
std::string WebRequest::parsePreferredAcceptValue(const char *str) const
{
  return std::string();
}
#endif // WT_NO_SPIRIT

WLocale WebRequest::parseLocale() const
{
  return WLocale(parsePreferredAcceptValue(headerValue("Accept-Language")));
}

void WebRequest::setAsyncCallback(const WriteCallback& cb)
{
  asyncCallback_ = cb;
}

const WebRequest::WriteCallback& WebRequest::getAsyncCallback()
{
  return asyncCallback_;
}

void WebRequest::emulateAsync(ResponseState state)
{
  if (async_) {
    if (state == ResponseState::ResponseDone)
      async_->done = true;

    return;
  }

  if (state == ResponseState::ResponseFlush) {
    async_ = new AsyncEmulation();

    /*
     * Invoke callback immediately and keep doing so until we get a
     * flush(ResponseDone). If we do not have a callback then the application
     * is waiting for some event to finish te request (e.g. a resource
     * continuation) and thus we exit now and wait for more flush()ing.
     */
    while (!async_->done) {
      if (!asyncCallback_) {
	delete async_;
	async_ = nullptr;
	return;
      }

      WriteCallback fn = asyncCallback_;
      asyncCallback_ = WriteCallback();
      fn(WebWriteEvent::Completed);
    }
  }
    
  delete this;
}

void WebRequest::setResponseType(ResponseType responseType)
{
  responseType_ = responseType;
}

const std::vector<std::pair<std::string, std::string> > &WebRequest::urlParams() const
{
  return urlParams_;
}

std::string WebRequest::clientAddress(const Configuration &conf) const
{
  std::string remoteAddr = str(envValue("REMOTE_ADDR"));
  if (conf.behindReverseProxy()) {
    // Old, deprecated behavior
    std::string clientIp = str(headerValue("Client-IP"));

    std::vector<std::string> ips;
    if (!clientIp.empty())
      boost::split(ips, clientIp, boost::is_any_of(","));

    std::string forwardedFor = str(headerValue("X-Forwarded-For"));

    std::vector<std::string> forwardedIps;
    if (!forwardedFor.empty())
      boost::split(forwardedIps, forwardedFor, boost::is_any_of(","));

    Utils::insert(ips, forwardedIps);

    for (auto &ip : ips) {
      boost::trim(ip);

      if (!ip.empty()
          && !isPrivateIP(ip)) {
        return ip;
      }
    }

    return remoteAddr;
  } else {
    if (conf.isTrustedProxy(remoteAddr)) {
      std::string forwardedFor = str(headerValue(conf.originalIPHeader().c_str()));
      boost::trim(forwardedFor);
      std::vector<std::string> forwardedIps;
      boost::split(forwardedIps, forwardedFor, boost::is_any_of(","));
      for (auto it = forwardedIps.rbegin();
           it != forwardedIps.rend(); ++it) {
        boost::trim(*it);
        if (!it->empty()) {
          if (!conf.isTrustedProxy(*it)) {
            return *it;
          } else {
            /*
             * When the left-most address in a forwardedHeader is contained
             * within a trustedProxy subnet, it should be returned as the clientAddress
             */
            remoteAddr = *it;
          }
        }
      }
    }
    return remoteAddr;
  }
}

}
