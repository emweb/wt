/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WebRequest.h"
#include "WtException.h"

#include <cstdlib>

#ifndef WT_NO_SPIRIT

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#include <boost/spirit.hpp>
#else
#include <boost/spirit/include/classic.hpp>
#endif

#include <boost/bind.hpp>

#endif // WT_NO_SPIRIT

using std::atoi;

namespace Wt {

Http::ParameterValues WebRequest::emptyValues_;

WebRequest::WebRequest()
  : entryPoint_(0),
    doingAsyncCallbacks_(false)
{ }

WebRequest::~WebRequest()
{ }

void WebRequest::readWebSocketMessage(CallbackFunction callback)
{ 
  throw WtException("should not get here");
}

bool WebRequest::webSocketMessagePending() const
{
  throw WtException("should not get here");
}

std::string WebRequest::userAgent() const
{
  return headerValue("User-Agent");
}

std::string WebRequest::referer() const
{
  return headerValue("Referer");
}

std::string WebRequest::contentType() const
{
  return envValue("CONTENT_TYPE");
}

int WebRequest::contentLength() const
{
  std::string lenstr = envValue("CONTENT_LENGTH");

  if (lenstr.empty())
    return 0;
  else
    return atoi(lenstr.c_str());
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

WT_LOCALE WebRequest::parsePreferredAcceptValue(const std::string& str) const
{
  std::vector<ValueListParser::Value> values;

  ValueListParser valueListParser(values);

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
WT_LOCALE WebRequest::parsePreferredAcceptValue(const std::string& str) const
{
  return std::string();
}
#endif // WT_NO_SPIRIT

WT_LOCALE WebRequest::parseLocale() const
{
  return parsePreferredAcceptValue(headerValue("Accept-Language"));
}

void WebRequest::setAsyncCallback(boost::function<void(void)> cb)
{
  asyncCallback_ = cb;
}

boost::function<void(void)> WebRequest::getAsyncCallback()
{
  return asyncCallback_;
}

void WebRequest::emulateAsync(ResponseState state)
{
  /*
   * This prevents stack build-up while emulating asynchronous callbacks
   * for a synchronous connector.
   */

  if (state == ResponseFlush) {
    if (doingAsyncCallbacks_) {
      // Do nothing. emulateAsync() was already called on this stack frame.
      // Unwind the stack and let the toplevel emulateAsync() call the cb.
    } else {
      doingAsyncCallbacks_ = true;

      while (asyncCallback_) {
	boost::function<void(void)> fn = asyncCallback_;
	asyncCallback_.clear();
	fn();
      };

      doingAsyncCallbacks_ = false;

      delete this;
    }
  } else {
    if (!doingAsyncCallbacks_)
      delete this;
    else {
      // we should in fact signal that we can delete after stopping the
      // asynccallbacks (e.g. by setting doingAsyncCallbacks_ = false
    }
  }
}

void WebRequest::setResponseType(ResponseType responseType)
{
  responseType_ = responseType;
}

}
