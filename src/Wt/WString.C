/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "3rdparty/rapidxml/rapidxml.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "Wt/WApplication"
#include "Wt/WServer"
#include "Wt/WString"
#include "Wt/WStringUtil"
#include "Wt/WWebWidget"
#include "Wt/WCombinedLocalizedStrings"

#include "WebUtils.h"

#ifndef WT_CNOR
namespace Wt {

std::vector<WString> WString::stArguments_;
const WString WString::Empty;
CharEncoding WString::defaultEncoding_ = Wt::LocalEncoding;

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
    if (realEncoding(encoding) == UTF8)
      utf8_ = value;
    else
      utf8_ = Wt::toUTF8(value);
  }
}

WString::WString(const std::string& value, CharEncoding encoding)
  : impl_(0)
{ 
  if (realEncoding(encoding) == UTF8)
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

WString::Impl::Impl()
  : n_(-1)
{ }

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

WString WString::trim() const
{
  std::string u8 = toUTF8();

  boost::trim(u8);

  return WString::fromUTF8(u8, false);
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
  unsigned pos = 0;
  for (; pos < value.length();) {
    unsigned at = pos;
    const char *c_start = value.c_str() + pos;
    const char *c = c_start;
    try {
      char *dest = 0;
      Wt::rapidxml::xml_document<>::copy_check_utf8(c, dest);
      pos += c - c_start;
    } catch (Wt::rapidxml::parse_error& e) {
      pos += c - c_start;
      for (unsigned i = at; i < pos && i < value.length();
        ++i)
	value[i] = '?';
    }
  }
}

std::string WString::resolveKey() const
{
  std::string result;
  WLocalizedStrings *ls = 0;

  WApplication *app = WApplication::instance();
  if (app)
    ls = app->localizedStrings_;

  if (!ls) {
    WServer *server = WServer::instance();
    if (server)
      ls = server->localizedStrings();
  }

  if (ls) {
    if (impl_->n_ == -1) {
      if (ls->resolveKey(impl_->key_, result))
	return result;
    } else {
      if (ls->resolvePluralKey(impl_->key_, result, impl_->n_))
	return result;
    }
  }

  return "??" + impl_->key_ + "??";
}

std::string WString::toUTF8() const
{
  if (impl_) {
    std::string result = utf8_;

    if (!impl_->key_.empty())
      result = resolveKey();

    for (unsigned i = 0; i < impl_->arguments_.size(); ++i) {
      std::string key = '{' + boost::lexical_cast<std::string>(i+1) + '}';
      Wt::Utils::replace(result, key, impl_->arguments_[i].toUTF8());
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

WString WString::trn(const char *key, ::uint64_t n)
{
  return WString(key, true, n);
}

WString WString::trn(const std::string& key, ::uint64_t n)
{
  return WString(key.c_str(), true, n);
}

WString::WString(const char *key, bool, ::uint64_t n)
{
  impl_ = new Impl;
  impl_->key_ = key;
  impl_->n_ = n;
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

  if (realEncoding(encoding) == UTF8)
    impl_->arguments_.push_back(WString::fromUTF8(value, true));
  else {
    WString s;
    s.utf8_ = Wt::toUTF8(value);
    impl_->arguments_.push_back(s);
  }

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

  WString s;
  s.utf8_ = Wt::toUTF8(value);
  impl_->arguments_.push_back(s);

  return *this;
}
#endif

WString& WString::arg(const WString& value)
{
  createImpl();

  impl_->arguments_.push_back(value);

  return *this;
}

WString& WString::arg(int value)
{
  return arg(WLocale::currentLocale().toString(value));
}

WString& WString::arg(unsigned value)
{
  return arg(WLocale::currentLocale().toString(value));
}

WString& WString::arg(::int64_t value)
{
  return arg(WLocale::currentLocale().toString(value));
}

WString& WString::arg(::uint64_t value)
{
  return arg(WLocale::currentLocale().toString(value));
}

WString& WString::arg(double value)
{
  return arg(WLocale::currentLocale().toString(value));
}

bool WString::refresh()
{
  if (literal())
    return false;
  else
    return true;
}

const std::vector<WString>& WString::args() const
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

void WString::setDefaultEncoding(Wt::CharEncoding encoding)
{
  if (encoding == DefaultEncoding)
    defaultEncoding_ = LocalEncoding;
  else
    defaultEncoding_ = encoding;
}

CharEncoding WString::realEncoding(CharEncoding encoding)
{
  return encoding == DefaultEncoding ? defaultEncoding_ : encoding;
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
    utf8_ = resolveKey();
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

