/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <map>
#include <stdio.h>
#include "Wt/WConfig.h"
#ifdef WT_THREADED
#include <thread>
#include <mutex>
#endif // WT_THREADED

#include <boost/algorithm/string/predicate.hpp>

#include "Wt/WAny.h"
#include "Wt/WDate.h"
#include "Wt/WDateTime.h"
#include "Wt/WLocalDateTime.h"
#include "Wt/WException.h"
#include "Wt/WLocale.h"
#include "Wt/WTime.h"
#include "Wt/WWebWidget.h"

#include "WebUtils.h"

#include <typeindex>

#ifdef WT_WIN32
#define snprintf _snprintf
#endif

namespace Wt {

LOGGER("WAbstractItemModel");

  namespace Impl {

#ifdef WT_THREADED
std::mutex registryMutex_;
#endif // WT_THREADED

typedef std::map<std::type_index,
		 std::shared_ptr<AbstractTypeHandler> > TypeRegistryMap;

typedef std::chrono::duration<int, std::milli> time_duration;

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

AbstractTypeHandler *getRegisteredType(const std::type_info &type,
				       bool takeLock)
{
  if (takeLock)
    lockTypeRegistry();

  TypeRegistryMap::iterator i = typeRegistry_.find(std::type_index(type));

  AbstractTypeHandler *result = nullptr;

  if (i != typeRegistry_.end())
    result = i->second.get();

  if (takeLock)
    unlockTypeRegistry();

  return result;
}

void registerType(const std::type_info &type, AbstractTypeHandler *handler)
{
  typeRegistry_[std::type_index(type)].reset(handler);
}

bool matchValue(const cpp17::any& value, const cpp17::any& query,
		WFlags<MatchFlag> flags)
{
  WFlags<MatchFlag> f = flags & MatchTypeMask;

  if ((f & MatchTypeMask) == MatchFlag::Exactly) {
    if (query.type() == value.type() ||
	(query.type() == typeid(WString) &&
	 value.type() == typeid(std::string)) ||
	(query.type() == typeid(std::string) &&
	 value.type() == typeid(WString)))
      return asString(query) == asString(value);
    else
      return false;
  } else {
    std::string query_str = asString(query).toUTF8();
    std::string value_str = asString(value).toUTF8();

    switch (f.value()) {
    case static_cast<int>(MatchFlag::StringExactly):
      return boost::iequals(value_str, query_str);
    case static_cast<int>(MatchFlag::StringExactly) | 
      static_cast<int>(MatchFlag::CaseSensitive):
      return boost::equals(value_str, query_str);

    case static_cast<int>(MatchFlag::StartsWith):
      return boost::istarts_with(value_str, query_str);
    case static_cast<int>(MatchFlag::StartsWith) |
      static_cast<int>(MatchFlag::CaseSensitive):
      return boost::starts_with(value_str, query_str);

    case static_cast<int>(MatchFlag::EndsWith):
      return boost::iends_with(value_str, query_str);
    case static_cast<int>(MatchFlag::EndsWith) | 
      static_cast<int>(MatchFlag::CaseSensitive):
      return boost::ends_with(value_str, query_str);

    default:
      throw WException("Not yet implemented: WAbstractItemModel::match with "
		       "MatchFlags = "
		       + std::to_string(flags.value()));
    }
  }
}

std::string asJSLiteral(const cpp17::any& v, TextFormat textFormat)
{
  if (!cpp17::any_has_value(v))
    return std::string("''");
  else if (v.type() == typeid(WString)) {
    WString s = cpp17::any_cast<WString>(v);

    bool plainText = false;
    if (textFormat == TextFormat::XHTML) {
      if (s.literal())
	plainText = !WWebWidget::removeScript(s);
    } else
      plainText = true;

    if (plainText && textFormat != TextFormat::UnsafeXHTML)
      s = WWebWidget::escapeText(s);

    return s.jsStringLiteral();
  } else if (v.type() == typeid(std::string)
	     || v.type() == typeid(const char *)) {
    WString s = v.type() == typeid(std::string) 
      ? WString::fromUTF8(cpp17::any_cast<std::string>(v))
      : WString::fromUTF8(cpp17::any_cast<const char *>(v));

    bool plainText;
    if (textFormat == TextFormat::XHTML)
      plainText = !WWebWidget::removeScript(s);
    else
      plainText = true;

    if (plainText && textFormat != TextFormat::UnsafeXHTML)
      s = WWebWidget::escapeText(s);

    return s.jsStringLiteral();
  } else if (v.type() == typeid(bool)) {
    bool b = cpp17::any_cast<bool>(v);
    return b ? "true" : "false";
  } else if (v.type() == typeid(WDate)) {
    const WDate& d = cpp17::any_cast<WDate>(v);

    return "new Date(" + std::to_string(d.year())
      + ',' + std::to_string(d.month() - 1)
      + ',' + std::to_string(d.day())
      + ')';
  } else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = cpp17::any_cast<WDateTime>(v);
    const WDate& d = dt.date();
    const WTime& t = dt.time();

    return "new Date(" + std::to_string(d.year())
      + ',' + std::to_string(d.month() - 1)
      + ',' + std::to_string(d.day())
      + ',' + std::to_string(t.hour())
      + ',' + std::to_string(t.minute())
      + ',' + std::to_string(t.second())
      + ',' + std::to_string(t.msec())
      + ')';
  } else if (v.type() == typeid(WLocalDateTime)) {
    const WLocalDateTime& dt = cpp17::any_cast<WLocalDateTime>(v);
    const WDate& d = dt.date();
    const WTime& t = dt.time();

    return "new Date(" + std::to_string(d.year())
      + ',' + std::to_string(d.month() - 1)
      + ',' + std::to_string(d.day())
      + ',' + std::to_string(t.hour())
      + ',' + std::to_string(t.minute())
      + ',' + std::to_string(t.second())
      + ',' + std::to_string(t.msec())
      + ')';
  }

#define ELSE_LEXICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return std::to_string(cpp17::any_cast<TYPE>(v))

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
    AbstractTypeHandler *handler = getRegisteredType(v.type(), true);
    if (handler)
      return handler->asString(v, WString::Empty).jsStringLiteral();
    else {
      LOG_ERROR("unsupported type: '"<< v.type().name() << "'");
      return "''";
    }
  }
}

template <class T>
T lexical_cast(const std::string& s) {
  std::stringstream ss;
  ss.str(s);
  T t;
  ss >> t;
  if (ss.fail())
    throw std::runtime_error("Could not cast " + s);
  return t;
}

cpp17::any updateFromJS(const cpp17::any& v, std::string s)
{
  if (!cpp17::any_has_value(v))
    return cpp17::any(s);
  else if (v.type() == typeid(WString))
    return cpp17::any(WString::fromUTF8(s));
  else if (v.type() == typeid(std::string))
    return cpp17::any(s);
  else if (v.type() == typeid(const char *))
    return cpp17::any(s);
  else if (v.type() == typeid(bool))
    return cpp17::any((s == "true" || s == "1") ? true : false);
  else if (v.type() == typeid(WDate))
    return cpp17::any(WDate::fromString(WString::fromUTF8(s),
					"ddd MMM d yyyy"));
  else if (v.type() == typeid(WDateTime))
    return cpp17::any(WDateTime::fromString(WString::fromUTF8(s),
					    "ddd MMM d yyyy HH:mm:ss"));
  else if (v.type() == typeid(WLocalDateTime))
    return cpp17::any(WLocalDateTime::fromString(WString::fromUTF8(s),
						 "ddd MMM d yyyy HH:mm:ss"));
#define ELSE_LEXICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return cpp17::any(lexical_cast<TYPE>(s))

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
    LOG_ERROR("unsupported type '" << v.type().name() << "'");
    return cpp17::any();
  }
}

int compare(const cpp17::any& d1, const cpp17::any& d2)
{
  const int UNSPECIFIED_RESULT = -1;

  /*
   * If the types are the same then we use std::operator< on that type
   * otherwise we compare lexicographically
   */
  if (cpp17::any_has_value(d1))
    if (cpp17::any_has_value(d2)) {
      if (d1.type() == d2.type()) {
	if (d1.type() == typeid(bool))
	  return static_cast<int>(cpp17::any_cast<bool>(d1))
	    - static_cast<int>(cpp17::any_cast<bool>(d2));

#define ELSE_COMPARE_ANY(TYPE)				\
	else if (d1.type() == typeid(TYPE)) {		\
	  TYPE v1 = cpp17::any_cast<TYPE>(d1);		\
	  TYPE v2 = cpp17::any_cast<TYPE>(d2);		\
	  return v1 == v2 ? 0 : (v1 < v2 ? -1 : 1);	\
        }

	ELSE_COMPARE_ANY(WString)
	ELSE_COMPARE_ANY(std::string)
	ELSE_COMPARE_ANY(WDate)
	ELSE_COMPARE_ANY(WDateTime)
	ELSE_COMPARE_ANY(WLocalDateTime)
    ELSE_COMPARE_ANY(std::chrono::system_clock::time_point)
    ELSE_COMPARE_ANY(time_duration)
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
	  AbstractTypeHandler *handler = getRegisteredType(d1.type(), true);
	  if (handler)
	    return handler->compare(d1, d2);
	  else {
	    LOG_ERROR("unsupported type '" << d1.type().name() << "'");
	    return 0;
	  }
	}
      } else {
	WString s1 = asString(d1);
	WString s2 = asString(d2);

	return s1 == s2 ? 0 : (s1 < s2 ? -1 : 1);
      }
    } else
      return -UNSPECIFIED_RESULT;
  else
    if (cpp17::any_has_value(d2))
      return UNSPECIFIED_RESULT;
    else
      return 0;
}

  }

WString asString(const cpp17::any& v, const WT_USTRING& format)
{
  if (!cpp17::any_has_value(v))
    return WString();
  else if (v.type() == typeid(WString))
    return cpp17::any_cast<WString>(v);
  else if (v.type() == typeid(std::string))
    return WString::fromUTF8(cpp17::any_cast<std::string>(v));
  else if (v.type() == typeid(const char *))
    return WString::fromUTF8(cpp17::any_cast<const char *>(v));
  else if (v.type() == typeid(bool))
    return WString::tr(cpp17::any_cast<bool>(v) ? "Wt.true" : "Wt.false");
  else if (v.type() == typeid(WDate)) {
    const WDate& d = cpp17::any_cast<WDate>(v);
    return d.toString(format.empty() ? 
		      WLocale::currentLocale().dateFormat() :
		      format);
  } else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = cpp17::any_cast<WDateTime>(v);
    return dt.toString(format.empty() ? 
			   WLocale::currentLocale().dateTimeFormat() : 
		       format);
  } else if (v.type() == typeid(WLocalDateTime)) {
    const WLocalDateTime& dt = cpp17::any_cast<WLocalDateTime>(v);
    return dt.toString();
  } else if (v.type() == typeid(WTime)) {
    const WTime& t = cpp17::any_cast<WTime>(v);
    return t.toString(format.empty() ? 
		WLocale::currentLocale().timeFormat() : 
		format);
  } else if(v.type() == typeid(std::chrono::system_clock::time_point)){
      const std::chrono::system_clock::time_point& tp
              = cpp17::any_cast<std::chrono::system_clock::time_point>(v);
      return WDateTime::fromTimePoint(tp)
              .toString(format.empty() ? WLocale::currentLocale().dateTimeFormat() : format);
  } else if(v.type() == typeid(std::chrono::duration<int, std::milli>)){
      const std::chrono::duration<int, std::milli>& ms
              = cpp17::any_cast<std::chrono::duration<int, std::milli>>(v);
      return WTime::fromTimeDuration(ms)
              .toString(format.empty() ? WLocale::currentLocale().timeFormat() : format);

  }

#define ELSE_LEXICAL_ANY(TYPE, CAST_TYPE)				\
  else if (v.type() == typeid(TYPE)) {					\
    if (format.empty())							\
      return WLocale::currentLocale()					\
        .toString((CAST_TYPE)cpp17::any_cast<TYPE>(v));			\
    else {								\
      char buf[100];							\
      snprintf(buf, 100, format.toUTF8().c_str(),			\
	       (CAST_TYPE)cpp17::any_cast<TYPE>(v));			\
      return WString::fromUTF8(buf);					\
    }									\
  }

  ELSE_LEXICAL_ANY(short, short)
  ELSE_LEXICAL_ANY(unsigned short, unsigned short)
  ELSE_LEXICAL_ANY(int, int)
  ELSE_LEXICAL_ANY(unsigned int, unsigned int)
  ELSE_LEXICAL_ANY(::int64_t, ::int64_t)
  ELSE_LEXICAL_ANY(::uint64_t, ::uint64_t)
  ELSE_LEXICAL_ANY(long long, ::int64_t)
  ELSE_LEXICAL_ANY(unsigned long long, ::uint64_t)
  ELSE_LEXICAL_ANY(float, float)
  ELSE_LEXICAL_ANY(double, double)

#undef ELSE_LEXICAL_ANY
  else if (v.type() == typeid(long)) {
    if (sizeof(long) == 4) {
      if (format.empty())
	return WLocale::currentLocale().toString
	  ((int)cpp17::any_cast<long>(v));
      else {
	char buf[100];
	snprintf(buf, 100, format.toUTF8().c_str(),
		 (int)cpp17::any_cast<long>(v));
	return WString::fromUTF8(buf);
      }
    } else {
      if (format.empty())
	return WLocale::currentLocale()
	  .toString((::int64_t)cpp17::any_cast<long>(v));
      else {
	char buf[100];
	snprintf(buf, 100, format.toUTF8().c_str(),
		 (::int64_t)cpp17::any_cast<long>(v));
	return WString::fromUTF8(buf);
      }
    }
  } else if (v.type() == typeid(unsigned long)) {
    if (sizeof(long) == 4) {
      if (format.empty())
	return WLocale::currentLocale().toString
	  ((unsigned)cpp17::any_cast<unsigned long>(v));
      else {
	char buf[100];
	snprintf(buf, 100, format.toUTF8().c_str(),
		 (unsigned)cpp17::any_cast<unsigned long>(v));
	return WString::fromUTF8(buf);
      }
    } else {
      if (format.empty())
	return WLocale::currentLocale()
	  .toString((::uint64_t)cpp17::any_cast<unsigned long>(v));
      else {
	char buf[100];
	snprintf(buf, 100, format.toUTF8().c_str(),
		 (::uint64_t)cpp17::any_cast<unsigned long>(v));
	return WString::fromUTF8(buf);
      }
    }
  }

  else {
    Impl::AbstractTypeHandler *handler = Impl::getRegisteredType(v.type(),
								 true);
    if (handler)
      return handler->asString(v, format);
    else {
      LOG_ERROR("unsupported type '" << v.type().name() << "'");
      return WString::Empty;
    }
  }
}

double asNumber(const cpp17::any& v)
{
  if (!cpp17::any_has_value(v))
    return std::numeric_limits<double>::signaling_NaN();
  else if (v.type() == typeid(WString))
    try {
      return WLocale::currentLocale().toDouble(cpp17::any_cast<WString>(v));
    } catch (std::exception& e) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(std::string))
    try {
      return WLocale::currentLocale().toDouble(WT_USTRING::fromUTF8(cpp17::any_cast<std::string>(v)));
    } catch (std::exception& e) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(const char *))
    try {
      return WLocale::currentLocale().toDouble(WT_USTRING::fromUTF8(cpp17::any_cast<const char *>(v)));
    } catch (std::exception&) {
      return std::numeric_limits<double>::signaling_NaN();
    }
  else if (v.type() == typeid(bool))
    return cpp17::any_cast<bool>(v) ? 1 : 0;
  else if (v.type() == typeid(WDate))
    return cpp17::any_cast<WDate>(v).toJulianDay();
  else if (v.type() == typeid(WDateTime)) {
    const WDateTime& dt = cpp17::any_cast<WDateTime>(v);
    return static_cast<double>(dt.toTime_t());
  } else if (v.type() == typeid(WLocalDateTime)) {
    const WLocalDateTime& dt = cpp17::any_cast<WLocalDateTime>(v);
    return static_cast<double>(dt.toUTC().toTime_t());
  } else if (v.type() == typeid(WTime)) {
    const WTime& t = cpp17::any_cast<WTime>(v);
    return static_cast<double>(WTime(0, 0).msecsTo(t));
  } else if(v.type() == typeid(std::chrono::system_clock::time_point)){
      const std::chrono::system_clock::time_point& tp
              = cpp17::any_cast<std::chrono::system_clock::time_point>(v);
      return static_cast<double>(WDateTime::fromTimePoint(tp).toTime_t());
  } else if (v.type() == typeid(std::chrono::duration<int, std::milli>)){
      const std::chrono::duration<int, std::milli>& dt
              = cpp17::any_cast<std::chrono::duration<int, std::milli>>(v);
      return static_cast<double>(dt.count());
  }

#define ELSE_NUMERICAL_ANY(TYPE) \
  else if (v.type() == typeid(TYPE)) \
    return static_cast<double>(cpp17::any_cast<TYPE>(v))

  ELSE_NUMERICAL_ANY(short);
  ELSE_NUMERICAL_ANY(unsigned short);
  ELSE_NUMERICAL_ANY(int);
  ELSE_NUMERICAL_ANY(unsigned int);
  ELSE_NUMERICAL_ANY(long);
  ELSE_NUMERICAL_ANY(unsigned long);
  ELSE_NUMERICAL_ANY(::int64_t);
  ELSE_NUMERICAL_ANY(::uint64_t);
  ELSE_NUMERICAL_ANY(long long);
  ELSE_NUMERICAL_ANY(float);
  ELSE_NUMERICAL_ANY(double);

#undef ELSE_NUMERICAL_ANY

  else {
    Impl::AbstractTypeHandler *handler = Impl::getRegisteredType(v.type(),
								 true);
    if (handler)
      return handler->asNumber(v);
    else {
      LOG_ERROR("unsupported type '" << v.type().name() << "'");
      return 0;
    }
  }
}

extern WT_API cpp17::any convertAnyToAny(const cpp17::any& v,
					 const std::type_info& type,
					 const WT_USTRING& format)
{
  if (!cpp17::any_has_value(v))
    return cpp17::any();
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
      (s, format.empty() ? 
       WLocale::currentLocale().dateFormat() :
       format);
  } else if (type == typeid(WDateTime)) {
    return WDateTime::fromString
      (s, format.empty() ? 
	   WLocale::currentLocale().dateTimeFormat() : format);
  } else if (type == typeid(WLocalDateTime)) {
    return WLocalDateTime::fromString(s);
  } else if (type == typeid(WTime)) {
    return WTime::fromString
      (s, format.empty() ? 
	   WLocale::currentLocale().timeFormat() :  format);
  } else if (type == typeid(std::chrono::system_clock::time_point)) {
    return WDateTime::fromString
      (s, format.empty() ? 
       WLocale::currentLocale().dateTimeFormat() : format).toTimePoint();
  } else if (type == typeid(std::chrono::duration<int, std::milli>)) {
    return WTime::fromString
      (s, format.empty() ?
       WLocale::currentLocale().timeFormat() : format).toTimeDuration();
  } else if (type == typeid(bool)) {
    std::string b = s.toUTF8();
    if (b == "true" || b == "1")
      return true;
    else if (b == "false" || b == "0")
      return false;
    else
      throw WException(std::string("Source string cannot be "
				   "converted to a bool value!"));
  }

  else if (type == typeid(short))
    return Utils::stoi(s.toUTF8());
  else if (type == typeid(unsigned short))
    return Utils::stoi(s.toUTF8());
  else if (type == typeid(int))
    return Utils::stoi(s.toUTF8());
  else if (type == typeid(unsigned int))
    return (unsigned int)Utils::stol(s.toUTF8());
  else if (type == typeid(long))
    return Utils::stol(s.toUTF8());
  else if (type == typeid(unsigned long))
    return (unsigned long)Utils::stoul(s.toUTF8());
  else if (type == typeid(::int64_t))
    return Utils::stoll(s.toUTF8());
  else if (type == typeid(::uint64_t))
    return (unsigned long)Utils::stoull(s.toUTF8());
  else if (type == typeid(long long))
    return Utils::stoll(s.toUTF8());
  else if (type == typeid(unsigned long long))
    return Utils::stoull(s.toUTF8());
  else if (type == typeid(float))
    return Utils::stof(s.toUTF8());
  else if (type == typeid(double))
    return Utils::stod(s.toUTF8());

#undef ELSE_LEXICAL_ANY

  else {
    // FIXME, add this to the traits.
    LOG_ERROR("unsupported type '" << v.type().name() << "'");
    return cpp17::any();
  }
}

}
