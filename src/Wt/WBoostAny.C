/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <stdio.h>

#ifdef WT_THREADED
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#endif // WT_THREADED

#include "WtException.h"

#include "Wt/WBoostAny"
#include "Wt/WDate"
#include "Wt/WDateTime"
#include "Wt/WTime"
#include "Wt/WWebWidget"

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace Wt {
  namespace Impl {

#ifdef WT_THREADED
boost::mutex registryMutex_;
#endif // WT_THREADED

typedef std::map<const std::type_info *,
		 boost::shared_ptr<AbstractTypeHandler> > TypeRegistryMap;

TypeRegistryMap typeRegistry_;

AbstractTypeHandler::AbstractTypeHandler()
{ }

AbstractTypeHandler::~AbstractTypeHandler()
{ }

void lockTypeRegistry()
{
#ifdef WT_THREADED
  registryMutex_.lock();
#endif // WT_THREADED
}

void unlockTypeRegistry()
{
#ifdef WT_THREADED
  registryMutex_.unlock();
#endif // WT_THREADED
}

AbstractTypeHandler *getRegisteredType(const std::type_info *type,
				       bool takeLock)
{
  if (takeLock)
    lockTypeRegistry();

  TypeRegistryMap::iterator i = typeRegistry_.find(type);

  AbstractTypeHandler *result = 0;

  if (i != typeRegistry_.end())
    result = i->second.get();

  if (takeLock)
    unlockTypeRegistry();

  return result;
}

void registerType(const std::type_info *type, AbstractTypeHandler *handler)
{
  typeRegistry_[type].reset(handler);
}

bool matchValue(const boost::any& value, const boost::any& query,
		WFlags<MatchFlag> flags)
{
  WFlags<MatchFlag> f = flags & MatchTypeMask;

  if ((f & MatchTypeMask) == MatchExactly)
    return (query.type() == value.type()) && asString(query) == asString(value);
  else {
    std::string query_str = asString(query).toUTF8();
    std::string value_str = asString(value).toUTF8();

    switch (f) {
    case MatchStringExactly:
      return boost::iequals(value_str, query_str);
    case MatchStringExactly | (int)MatchCaseSensitive:
      return boost::equals(value_str, query_str);

    case MatchStartsWith:
      return boost::istarts_with(value_str, query_str);
    case MatchStartsWith | (int)MatchCaseSensitive:
      return boost::starts_with(value_str, query_str);

    case MatchEndsWith:
      return boost::iends_with(value_str, query_str);
    case MatchEndsWith | (int)MatchCaseSensitive:
      return boost::ends_with(value_str, query_str);

    default:
      throw WtException("Not yet implemented: WAbstractItemModel::match with "
			"MatchFlags = "
			+ boost::lexical_cast<std::string>(flags));
    }
  }
}

std::string asJSLiteral(const boost::any& v, bool xhtml)
{
  if (v.empty())
    return std::string("''");
  else if (v.type() == typeid(WString)) {
    WString s = boost::any_cast<WString>(v);

    bool plainText = false;
    if (xhtml) {
      if (s.literal())
	plainText = !WWebWidget::removeScript(s);
    } else
      plainText = true;

    if (plainText)
      s = WWebWidget::escapeText(s);

    return s.jsStringLiteral();
  } else if (v.type() == typeid(std::string)
	     || v.type() == typeid(const char *)) {
    WString s = v.type() == typeid(std::string) 
      ? WString::fromUTF8(boost::any_cast<std::string>(v))
      : WString::fromUTF8(boost::any_cast<const char *>(v));

    bool plainText;
    if (xhtml)
      plainText = !WWebWidget::removeScript(s);
    else
      plainText = true;

    if (plainText)
      s = WWebWidget::escapeText(s);

    return s.jsStringLiteral();
  } else if (v.type() == typeid(WDate)) {
    const WDate& d = boost::any_cast<WDate>(v);

    return "new Date(" + boost::lexical_cast<std::string>(d.year())
      + ',' + boost::lexical_cast<std::string>(d.month() - 1)
      + ',' + boost::lexical_cast<std::string>(d.day())
      + ')';
  } else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = boost::any_cast<WDateTime>(v);
    const WDate& d = dt.date();
    const WTime& t = dt.time();

    return "new Date(" + boost::lexical_cast<std::string>(d.year())
      + ',' + boost::lexical_cast<std::string>(d.month() - 1)
      + ',' + boost::lexical_cast<std::string>(d.day())
      + ',' + boost::lexical_cast<std::string>(t.hour())
      + ',' + boost::lexical_cast<std::string>(t.minute())
      + ',' + boost::lexical_cast<std::string>(t.second())
      + ',' + boost::lexical_cast<std::string>(t.msec())
      + ')';
  }

#define ELSE_LEXICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return boost::lexical_cast<std::string>(boost::any_cast<TYPE>(v))

  ELSE_LEXICAL_ANY(short);
  ELSE_LEXICAL_ANY(unsigned short);
  ELSE_LEXICAL_ANY(int);
  ELSE_LEXICAL_ANY(unsigned int);
  ELSE_LEXICAL_ANY(long);
  ELSE_LEXICAL_ANY(unsigned long);
  ELSE_LEXICAL_ANY(::int64_t);
  ELSE_LEXICAL_ANY(::uint64_t);
  ELSE_LEXICAL_ANY(long long);
  ELSE_LEXICAL_ANY(unsigned long long);
  ELSE_LEXICAL_ANY(float);
  ELSE_LEXICAL_ANY(double);

#undef ELSE_LEXICAL_ANY

  else {
    AbstractTypeHandler *handler = getRegisteredType(&v.type(), true);
    if (handler)
      return handler->asString(v, WString::Empty).jsStringLiteral();
    else
      throw WtException(std::string("WAbstractItemModel: unsupported type ")
			+ v.type().name());
  }
}

boost::any updateFromJS(const boost::any& v, std::string s)
{
  if (v.empty())
    return boost::any(s);
  else if (v.type() == typeid(WString))
    return boost::any(WString::fromUTF8(s));
  else if (v.type() == typeid(std::string))
    return boost::any(s);
  else if (v.type() == typeid(const char *))
    return boost::any(s);
  else if (v.type() == typeid(WDate))
    return boost::any(WDate::fromString(WString::fromUTF8(s),
					"ddd MMM d yyyy"));
  else if (v.type() == typeid(WDateTime))
    return boost::any(WDateTime::fromString(WString::fromUTF8(s),
					    "ddd MMM d yyyy HH:mm:ss"));
#define ELSE_LEXICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return boost::any(boost::lexical_cast<TYPE>(s))

  ELSE_LEXICAL_ANY(short);
  ELSE_LEXICAL_ANY(unsigned short);
  ELSE_LEXICAL_ANY(int);
  ELSE_LEXICAL_ANY(unsigned int);
  ELSE_LEXICAL_ANY(long);
  ELSE_LEXICAL_ANY(unsigned long);
  ELSE_LEXICAL_ANY(int64_t);
  ELSE_LEXICAL_ANY(uint64_t);
  ELSE_LEXICAL_ANY(long long);
  ELSE_LEXICAL_ANY(unsigned long long);
  ELSE_LEXICAL_ANY(float);
  ELSE_LEXICAL_ANY(double);

#undef ELSE_LEXICAL_ANY

  else
    throw WtException(std::string("WAbstractItemModel: unsupported type ")
		      + v.type().name());
}

int compare(const boost::any& d1, const boost::any& d2)
{
  const int UNSPECIFIED_RESULT = -1;

  /*
   * If the types are the same then we use std::operator< on that type
   * otherwise we compare lexicographically
   */
  if (!d1.empty())
    if (!d2.empty()) {
      if (d1.type() == d2.type()) {
	if (d1.type() == typeid(bool))
	  return static_cast<int>(boost::any_cast<bool>(d1))
	    - static_cast<int>(boost::any_cast<bool>(d2));

#define ELSE_COMPARE_ANY(TYPE)				\
	else if (d1.type() == typeid(TYPE)) {		\
	  TYPE v1 = boost::any_cast<TYPE>(d1);		\
	  TYPE v2 = boost::any_cast<TYPE>(d2);		\
	  return v1 == v2 ? 0 : (v1 < v2 ? -1 : 1);	\
        }

	ELSE_COMPARE_ANY(WString)
	ELSE_COMPARE_ANY(std::string)
	ELSE_COMPARE_ANY(WDate)
	ELSE_COMPARE_ANY(WDateTime)
	ELSE_COMPARE_ANY(WTime)
	ELSE_COMPARE_ANY(short)
	ELSE_COMPARE_ANY(unsigned short)
	ELSE_COMPARE_ANY(int)
	ELSE_COMPARE_ANY(unsigned int)
	ELSE_COMPARE_ANY(long)
	ELSE_COMPARE_ANY(unsigned long)
	ELSE_COMPARE_ANY(::int64_t)
	ELSE_COMPARE_ANY(::uint64_t)
	ELSE_COMPARE_ANY(long long)
	ELSE_COMPARE_ANY(unsigned long long)
	ELSE_COMPARE_ANY(float)
	ELSE_COMPARE_ANY(double)

#undef ELSE_COMPARE_ANY
	else {
	  AbstractTypeHandler *handler = getRegisteredType(&d1.type(), true);
	  if (handler)
	    return handler->compare(d1, d2);
	  else	  
	    throw WtException(std::string
			      ("WAbstractItemModel: unsupported type ")
			      + d1.type().name());
	}
      } else {
	WString s1 = asString(d1);
	WString s2 = asString(d2);

	return s1 == s2 ? 0 : (s1 < s2 ? -1 : 1);
      }
    } else
      return -UNSPECIFIED_RESULT;
  else
    if (!d2.empty())
      return UNSPECIFIED_RESULT;
    else
      return 0;
}

  }

WString asString(const boost::any& v, const WT_USTRING& format)
{
  if (v.empty())
    return WString();
  else if (v.type() == typeid(WString))
    return boost::any_cast<WString>(v);
  else if (v.type() == typeid(std::string))
    return WString::fromUTF8(boost::any_cast<std::string>(v));
  else if (v.type() == typeid(const char *))
    return WString::fromUTF8(boost::any_cast<const char *>(v));
  else if (v.type() == typeid(WDate)) {
    const WDate& d = boost::any_cast<WDate>(v);
    return d.toString(format.empty() ? "dd/MM/yy" : format);
  } else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = boost::any_cast<WDateTime>(v);
    return dt.toString(format.empty() ? "dd/MM/yy HH:mm:ss" : format);
  } else if (v.type() == typeid(WTime)) {
    const WTime& t = boost::any_cast<WTime>(v);
    return t.toString(format.empty() ? "HH:mm:ss" : format);
  }

#define ELSE_LEXICAL_ANY(TYPE)						\
  else if (v.type() == typeid(TYPE)) {					\
    if (format.empty())							\
      return WString::fromUTF8(boost::lexical_cast<std::string>		\
			       (boost::any_cast<TYPE>(v)));		\
    else {								\
      char buf[100];							\
      snprintf(buf, 100, format.toUTF8().c_str(), boost::any_cast<TYPE>(v)); \
      return WString::fromUTF8(buf);					\
    }									\
  }

  ELSE_LEXICAL_ANY(short)
  ELSE_LEXICAL_ANY(unsigned short)
  ELSE_LEXICAL_ANY(int)
  ELSE_LEXICAL_ANY(unsigned int)
  ELSE_LEXICAL_ANY(long)
  ELSE_LEXICAL_ANY(unsigned long)
  ELSE_LEXICAL_ANY(int64_t)
  ELSE_LEXICAL_ANY(uint64_t)
  ELSE_LEXICAL_ANY(long long)
  ELSE_LEXICAL_ANY(float)
  ELSE_LEXICAL_ANY(double)

#undef ELSE_LEXICAL_ANY

  else {
    Impl::AbstractTypeHandler *handler = Impl::getRegisteredType(&v.type(),
								 true);
    if (handler)
      return handler->asString(v, format);
    else	  
      throw WtException(std::string("WAbstractItemModel: unsupported type ")
			+ v.type().name());
  }
}

double asNumber(const boost::any& v)
{
  if (v.empty())
    return std::numeric_limits<double>::signaling_NaN();
  else if (v.type() == typeid(WString))
    try {
      return boost::lexical_cast<double>(boost::any_cast<WString>(v).toUTF8());
    } catch (boost::bad_lexical_cast& e) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(std::string))
    try {
      return boost::lexical_cast<double>(boost::any_cast<std::string>(v));
    } catch (boost::bad_lexical_cast& e) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(const char *))
    try {
      return boost::lexical_cast<double>(boost::any_cast<const char *>(v));
    } catch (boost::bad_lexical_cast&) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(WDate))
    return static_cast<double>(boost::any_cast<WDate>(v).toJulianDay());
  else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = boost::any_cast<WDateTime>(v);
    return static_cast<double>(dt.toTime_t());
  } else if (v.type() == typeid(WTime)) {
    const WTime& t = boost::any_cast<WTime>(v);
    return static_cast<double>(WTime(0, 0).msecsTo(t));
  }

#define ELSE_NUMERICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return static_cast<double>(boost::any_cast<TYPE>(v))

  ELSE_NUMERICAL_ANY(short);
  ELSE_NUMERICAL_ANY(unsigned short);
  ELSE_NUMERICAL_ANY(int);
  ELSE_NUMERICAL_ANY(unsigned int);
  ELSE_NUMERICAL_ANY(long);
  ELSE_NUMERICAL_ANY(unsigned long);
  ELSE_NUMERICAL_ANY(int64_t);
  ELSE_NUMERICAL_ANY(uint64_t);
  ELSE_NUMERICAL_ANY(long long);
  ELSE_NUMERICAL_ANY(float);
  ELSE_NUMERICAL_ANY(double);

#undef ELSE_NUMERICAL_ANY

  else {
    Impl::AbstractTypeHandler *handler = Impl::getRegisteredType(&v.type(),
								 true);
    if (handler)
      return handler->asNumber(v);
    else	  
      throw WtException(std::string("WAbstractItemModel: unsupported type ")
			+ v.type().name());
  }
}

extern WT_API boost::any convertAnyToAny(const boost::any& v,
					 const std::type_info& type,
					 const WT_USTRING& format)
{
  if (v.empty())
    return boost::any();
  else if (v.type() == type)
    return v;

  WString s = asString(v, format);

  if (type == typeid(WString))
    return s;
  else if (type == typeid(std::string))
    return s.toUTF8();
  else if (type == typeid(const char *))
    return s.toUTF8().c_str();
  else if (type == typeid(WDate)) {
    return WDate::fromString
      (s, format.empty() ? "dd/MM/yy" : format);
  } else if (v.type() == typeid(WDateTime)) {
    return WDateTime::fromString
      (s, format.empty() ? "dd/MM/yy HH:mm:ss" : format);
  } else if (v.type() == typeid(WTime)) {
    return WTime::fromString
      (s, format.empty() ? "HH:mm:ss" : format);
  }

#define ELSE_LEXICAL_ANY(TYPE)						\
  else if (v.type() == typeid(TYPE)) {					\
    return boost::lexical_cast<TYPE>(s.toUTF8());			\
  }

  ELSE_LEXICAL_ANY(short)
  ELSE_LEXICAL_ANY(unsigned short)
  ELSE_LEXICAL_ANY(int)
  ELSE_LEXICAL_ANY(unsigned int)
  ELSE_LEXICAL_ANY(long)
  ELSE_LEXICAL_ANY(unsigned long)
  ELSE_LEXICAL_ANY(int64_t)
  ELSE_LEXICAL_ANY(uint64_t)
  ELSE_LEXICAL_ANY(long long)
  ELSE_LEXICAL_ANY(float)
  ELSE_LEXICAL_ANY(double)

#undef ELSE_LEXICAL_ANY

  else {
    // FIXME, add this to the traits.
    throw WtException(std::string("WAbstractItemModel: unsupported type ")
		      + v.type().name());
  }
}

}
