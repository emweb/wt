/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "rapidxml/rapidxml.hpp"
#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WString"
#include "Wt/WStringUtil"
#include "Wt/WWebWidget"
#include "Wt/WCombinedLocalizedStrings"

#include "WtException.h"
#include "Utils.h"

#ifndef WT_CNOR
namespace Wt {

std::vector<std::string> WString::stArguments_;
const WString WString::Empty;

WString::WString()
  : impl_(0)
{ }

#ifndef WT_NO_STD_WSTRING
WString::WString(const wchar_t *value)
  : impl_(0)
{ 
  if (value)
    utf8_ = Wt::toUTF8(value);
}
#endif

#ifndef WT_NO_STD_WSTRING
WString::WString(const std::wstring& value)
  : impl_(0)
{ 
  utf8_ = Wt::toUTF8(value);
}
#endif

WString::WString(const char *value, CharEncoding encoding)
  : impl_(0)
{
  if (value) {
    if (encoding == UTF8)
      utf8_ = value;
    else
      utf8_ = Wt::toUTF8(value);
  }
}

WString::WString(const std::string& value, CharEncoding encoding)
  : impl_(0)
{ 
  if (encoding == UTF8)
    utf8_ = value;
  else
    utf8_ = Wt::toUTF8(value);
}

#ifndef WT_NO_STD_LOCALE
WString::WString(const char *value, const std::locale& loc)
  : impl_(0)
{
  utf8_ = Wt::toUTF8(value, loc);
}
#endif

#ifndef WT_NO_STD_LOCALE
WString::WString(const std::string& value, const std::locale& loc)
  : impl_(0)
{
  utf8_ = Wt::toUTF8(value, loc);
}
#endif

WString::WString(const WString& other)
  : utf8_(other.utf8_),
    impl_(0)
{
  if (other.impl_)
    impl_ = new Impl(*other.impl_);
}

WString::~WString()
{
  delete impl_;
}

bool WString::operator== (const WString& rhs) const
{
  return toUTF8() == rhs.toUTF8();
}

bool WString::operator< (const WString& rhs) const
{
  return toUTF8() < rhs.toUTF8();
}

bool WString::operator> (const WString& rhs) const
{
  return toUTF8() > rhs.toUTF8();
}

WString& WString::operator= (const WString& rhs)
{
  if (this != &rhs) {
    this->WString::~WString();
    new (this) WString(rhs);
  }

  return *this;
}

WString& WString::operator+= (const WString& rhs)
{
  makeLiteral();
  utf8_ += rhs.toUTF8();

  return *this;
}

#ifndef WT_NO_STD_WSTRING
WString& WString::operator+= (const std::wstring& rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}
#endif

#ifndef WT_NO_STD_WSTRING
WString& WString::operator+= (const wchar_t *rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}
#endif

WString& WString::operator+= (const std::string& rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

WString& WString::operator+= (const char *rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

bool WString::empty() const
{
  if (literal())
    return utf8_.empty();
  else
    return toUTF8().empty();
}

WString WString::fromUTF8(const std::string& value, bool checkValid)
{
  WString result(value, UTF8);
  if (checkValid)
    checkUTF8Encoding(result.utf8_);
  return result;
}

WString WString::fromUTF8(const char *value, bool checkValid)
{
  WString result(value, UTF8);
  if (checkValid)
    checkUTF8Encoding(result.utf8_);
  return result;
}

void WString::checkUTF8Encoding(std::string& value)
{
  const char *c = value.c_str();
  for (; c != value.c_str() + value.length();) {
    const char *at = c;
    try {
      char *dest = 0;
      rapidxml::xml_document<>::copy_check_utf8(c, dest);
    } catch (rapidxml::parse_error& e) {
      for (const char *i = at; i < c; ++i)
	value[i - value.c_str()] = '?';
    }
  }
}

void WString::resolveKey(const std::string& key, std::string& result) const
{
  if ((!wApp) ||
      !wApp->localizedStrings_->resolveKey(impl_->key_, result))
    result = "??" + key + "??";
}

std::string WString::toUTF8() const
{
  if (impl_) {
    std::string result = utf8_;

    if (!impl_->key_.empty())
      resolveKey(impl_->key_, result);

    for (unsigned i = 0; i < impl_->arguments_.size(); ++i) {
      std::string key = '{' + boost::lexical_cast<std::string>(i+1) + '}';
      Wt::Utils::replace(result, key, impl_->arguments_[i]);
    }

    return result;
  } else
    return utf8_;
}

WString WString::tr(const char *key)
{
  return WString(key, true);
}

WString WString::tr(const std::string& key)
{
  return WString(key.c_str(), true);
}

WString::WString(const char *key, bool)
{
  impl_ = new Impl;
  impl_->key_ = key;
}

#ifndef WT_NO_STD_WSTRING
std::wstring WString::value() const
{
  return Wt::fromUTF8(toUTF8());
}
#endif

#ifndef WT_NO_STD_LOCALE
std::string WString::narrow(const std::locale &loc) const
{
  return Wt::fromUTF8(toUTF8(), loc);
}
#else
std::string WString::narrow() const
{
  return Wt::fromUTF8(toUTF8(), LocalEncoding);
}
#endif

#ifndef WT_NO_STD_WSTRING
WString::operator std::wstring() const
{
  return value();
}
#endif

const std::string WString::key() const
{
  if (impl_)
    return impl_->key_;
  else
    return std::string();
}

void WString::createImpl()
{
  if (!impl_)
    impl_ = new Impl();
}

WString& WString::arg(const std::string& value, CharEncoding encoding)
{
  createImpl();

  if (encoding == UTF8) {
    impl_->arguments_.push_back(value);
    checkUTF8Encoding(impl_->arguments_.back());
  } else
    impl_->arguments_.push_back(Wt::toUTF8(value));

  return *this;
}

WString& WString::arg(const char *value, CharEncoding encoding)
{
  return arg(std::string(value), encoding);
}

#ifndef WT_NO_STD_WSTRING
WString& WString::arg(const std::wstring& value)
{
  createImpl();

  impl_->arguments_.push_back(Wt::toUTF8(value));

  return *this;
}
#endif

WString& WString::arg(const WString& value)
{
  createImpl();

  impl_->arguments_.push_back(value.toUTF8());

  return *this;
}

WString& WString::arg(int value)
{
  createImpl();

  impl_->arguments_.push_back(boost::lexical_cast<std::string>(value));

  return *this;
}

WString& WString::arg(double value)
{
  createImpl();

  impl_->arguments_.push_back(boost::lexical_cast<std::string>(value));

  return *this;
}

bool WString::refresh()
{
  if (literal())
    return false;
  else
    return true;
}

const std::vector<std::string>& WString::args() const
{
  if (impl_)
    return impl_->arguments_;
  else
    return stArguments_;
}

WString utf8(const char *value)
{
  return WString(value, UTF8);
}

WString utf8(const std::string& value)
{
  return WString(value, UTF8);
}

std::string WString::jsStringLiteral(char delimiter) const
{
  return WWebWidget::jsStringLiteral(toUTF8(), delimiter);
}

WString operator+ (const WString& lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

#ifndef WT_NO_STD_WSTRING
WString operator+ (const WString& lhs, const std::wstring& rhs)
{
  WString result = lhs;
  return result += rhs;
}
#endif

#ifndef WT_NO_STD_WSTRING
WString operator+ (const WString& lhs, const wchar_t *rhs)
{
  WString result = lhs;
  return result += rhs;
}
#endif

WString operator+ (const WString& lhs, const std::string& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const char *rhs)
{
  WString result = lhs;
  return result += rhs;
}

#ifndef WT_NO_STD_WSTRING
WString operator+ (const std::wstring& lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}
#endif

#ifndef WT_NO_STD_WSTRING
WString operator+ (const wchar_t *lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}
#endif

WString operator+ (const std::string& lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const char *lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

bool operator== (const char *lhs, const WString& rhs)
{
  return rhs == lhs;
}

bool operator== (const std::string& lhs, const WString& rhs)
{
  return rhs == lhs;
}

#ifndef WT_NO_STD_WSTRING
bool operator== (const std::wstring& lhs, const WString& rhs)
{
  return rhs == lhs;
}
#endif

#ifndef WT_NO_STD_WSTRING
bool operator== (const wchar_t *lhs, const WString& rhs)
{
  return rhs == lhs;
}
#endif


bool operator!= (const char *lhs, const WString& rhs)
{
  return !(rhs == lhs);
}

bool operator!= (const std::string& lhs, const WString& rhs)
{
  return !(rhs == lhs);
}

#ifndef WT_NO_STD_WSTRING
bool operator!= (const std::wstring& lhs, const WString& rhs)
{
  return !(rhs == lhs);
}
#endif

#ifndef WT_NO_STD_WSTRING
bool operator!= (const wchar_t *lhs, const WString& rhs)
{
  return !(rhs == lhs);
}
#endif

void WString::makeLiteral()
{
  if (!literal()) {
    resolveKey(impl_->key_, utf8_);
    impl_->key_ = std::string();
  }
}

#ifndef WT_NO_STD_WSTRING
std::wostream& operator<< (std::wostream& lhs, const WString& rhs)
{
  return lhs << rhs.value();
}
#endif

std::ostream& operator<< (std::ostream& lhs, const WString& rhs)
{
  return lhs << rhs.narrow();
}

}

#endif // WT_CNOR

