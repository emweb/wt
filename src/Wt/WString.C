/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "3rdparty/rapidxml/rapidxml.hpp"

#include <boost/algorithm/string/trim.hpp>

#include "Wt/WApplication.h"
#include "Wt/WServer.h"
#include "Wt/WString.h"
#include "Wt/WStringUtil.h"
#include "Wt/WWebWidget.h"
#include "Wt/WCombinedLocalizedStrings.h"

#include "WebUtils.h"

#ifndef WT_CNOR
namespace Wt {

std::vector<WString> WString::stArguments_;
const WString WString::Empty;
CharEncoding WString::defaultEncoding_ = Wt::CharEncoding::UTF8;

WString::WString()
  : impl_(nullptr)
{ }

WString::WString(const wchar_t *value)
  : impl_(nullptr)
{ 
  if (value)
    utf8_ = Wt::toUTF8(value);
}

WString::WString(const std::wstring& value)
  : impl_(nullptr)
{ 
  utf8_ = Wt::toUTF8(value);
}

WString::WString(const char16_t *value)
  : impl_(nullptr)
{
  if (value)
    utf8_ = Wt::toUTF8(value);
}

WString::WString(const std::u16string &value)
  : impl_(nullptr)
{
  utf8_ = Wt::toUTF8(value);
}

WString::WString(const char32_t *value)
  : impl_(nullptr)
{
  if (value)
    utf8_ = Wt::toUTF8(value);
}

WString::WString(const std::u32string &value)
  : impl_(nullptr)
{
  utf8_ = Wt::toUTF8(value);
}

WString::WString(const char *value, CharEncoding encoding)
  : impl_(nullptr)
{
  if (value) {
    if (realEncoding(encoding) == CharEncoding::UTF8)
      utf8_ = value;
    else
      utf8_ = Wt::toUTF8(value);
  }
}

WString::WString(const std::string& value, CharEncoding encoding)
  : impl_(nullptr)
{ 
  if (realEncoding(encoding) == CharEncoding::UTF8)
    utf8_ = value;
  else
    utf8_ = Wt::toUTF8(value);
}

WString::WString(std::string&& value, CharEncoding encoding)
  : impl_(nullptr)
{
  if (realEncoding(encoding) == CharEncoding::UTF8)
    utf8_ = std::move(value);
  else {
    utf8_ = Wt::toUTF8(value);
    value.clear();
  }
}

WString::WString(const char *value, const std::locale& loc)
  : impl_(nullptr)
{
  utf8_ = Wt::toUTF8(value, loc);
}

WString::WString(const std::string& value, const std::locale& loc)
  : impl_(nullptr)
{
  utf8_ = Wt::toUTF8(value, loc);
}

WString::WString(const WString& other)
  : utf8_(other.utf8_),
    impl_(nullptr)
{
  if (other.impl_)
    impl_ = new Impl(*other.impl_);
}

WString::WString(WString&& other)
  : utf8_(std::move(other.utf8_)),
    impl_(other.impl_)
{
  other.impl_ = nullptr;
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

WString& WString::operator= (WString&& rhs)
{
  if (this != &rhs) {
    this->WString::~WString();
    new (this) WString(std::move(rhs));
  }

  return *this;
}

WString& WString::operator+= (const WString& rhs)
{
  makeLiteral();
  utf8_ += rhs.toUTF8();

  return *this;
}

WString& WString::operator+= (const std::wstring& rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

WString& WString::operator+= (const std::u16string& rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

WString& WString::operator+= (const std::u32string& rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

WString& WString::operator+= (const wchar_t *rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

WString& WString::operator+= (const char16_t *rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

WString& WString::operator+= (const char32_t *rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(rhs);

  return *this;
}

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
  WString result(value, CharEncoding::UTF8);
  if (checkValid)
    checkUTF8Encoding(result.utf8_);
  return result;
}

WString WString::fromUTF8(std::string&& value, bool checkValid)
{
  WString result(std::move(value), CharEncoding::UTF8);
  if (checkValid)
    checkUTF8Encoding(result.utf8_);
  return result;
}

WString WString::fromUTF8(const char *value, bool checkValid)
{
  WString result(value, CharEncoding::UTF8);
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
      char *dest = nullptr;
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

std::string WString::resolveKey(TextFormat format) const
{
  LocalizedString result;
  WLocalizedStrings *ls = nullptr;
  const WLocale *locale = nullptr;

  WApplication *app = WApplication::instance();
  if (app) {
    ls = app->localizedStringsPack();
    locale = &WLocale::currentLocale();
  }

  if (!ls) {
    WServer *server = WServer::instance();
    if (server) {
      ls = server->localizedStrings().get();
      locale = &WLocale::currentLocale();
    }
  }

  if (ls) {
    if (impl_->n_ == -1) {
      result = ls->resolveKey(*locale, impl_->key_);
    } else {
      result = ls->resolvePluralKey(*locale, impl_->key_, impl_->n_);
    }
  }

  if (!result) {
    result = LocalizedString{"??" + impl_->key_ + "??", TextFormat::Plain};
  }

  if (result.format == format)
    return result.value;
  else if (result.format == TextFormat::Plain && format != TextFormat::Plain) {
    return Wt::WWebWidget::escapeText(result.value);
  } else {
    return Wt::WWebWidget::unescapeText(result.value);
  }
}

std::string WString::toUTF8() const
{
  if (impl_) {
    std::string result = utf8_;

    if (!impl_->key_.empty())
      result = resolveKey(TextFormat::Plain);

    for (unsigned i = 0; i < impl_->arguments_.size(); ++i) {
      std::string key = '{' + std::to_string(i+1) + '}';
      Wt::Utils::replace(result, key, impl_->arguments_[i].toUTF8());
    }

    return result;
  } else
    return utf8_;
}

std::string WString::toXhtmlUTF8() const
{
  if (impl_) {
    std::string result = utf8_;

    if (!impl_->key_.empty())
      result = resolveKey(TextFormat::XHTML);

    for (unsigned i = 0; i < impl_->arguments_.size(); ++i) {
      std::string key = '{' + std::to_string(i+1) + '}';
      Wt::Utils::replace(result, key, impl_->arguments_[i].toXhtmlUTF8());
    }

    return result;
  } else {
    return utf8_;
  }
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

std::wstring WString::value() const
{
  return Wt::fromUTF8(toUTF8());
}

std::u16string WString::toUTF16() const
{
  return Wt::utf8ToUTF16(toUTF8());
}

std::u32string WString::toUTF32() const
{
  return Wt::utf8ToUTF32(toUTF8());
}

std::string WString::narrow(const std::locale &loc) const
{
  return Wt::fromUTF8(toUTF8(), loc);
}

WString::operator std::wstring() const
{
  return value();
}

WString::operator std::u16string() const
{
  return toUTF16();
}

WString::operator std::u32string() const
{
  return toUTF32();
}

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

  if (realEncoding(encoding) == CharEncoding::UTF8)
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

WString& WString::arg(const std::wstring& value)
{
  createImpl();

  WString s;
  s.utf8_ = Wt::toUTF8(value);
  impl_->arguments_.push_back(s);

  return *this;
}

WString& WString::arg(const wchar_t *value)
{
  return arg(std::wstring(value));
}

WString& WString::arg(const std::u16string& value)
{
  createImpl();

  WString s;
  s.utf8_ = Wt::toUTF8(value);
  impl_->arguments_.push_back(s);

  return *this;
}

WString& WString::arg(const char16_t *value)
{
  return arg(std::u16string(value));
}

WString& WString::arg(const std::u32string& value)
{
  createImpl();

  WString s;
  s.utf8_ = Wt::toUTF8(value);
  impl_->arguments_.push_back(s);

  return *this;
}

WString& WString::arg(const char32_t *value)
{
  return arg(std::u32string(value));
}

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

WString& WString::arg(long value)
{
  return arg(WLocale::currentLocale().toString(value));
}

WString& WString::arg(unsigned long value)
{
  return arg(WLocale::currentLocale().toString(value));
}

WString& WString::arg(long long value)
{
  return arg(WLocale::currentLocale().toString(value));
}

WString& WString::arg(unsigned long long value)
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
  return WString(value, CharEncoding::UTF8);
}

WString utf8(const std::string& value)
{
  return WString(value, CharEncoding::UTF8);
}

std::string WString::jsStringLiteral(char delimiter) const
{
  return WWebWidget::jsStringLiteral(toUTF8(), delimiter);
}

void WString::setDefaultEncoding(Wt::CharEncoding encoding)
{
  if (encoding == CharEncoding::Default)
    defaultEncoding_ = CharEncoding::Local;
  else
    defaultEncoding_ = encoding;
}

CharEncoding WString::realEncoding(CharEncoding encoding)
{
  return encoding == CharEncoding::Default 
    ? defaultEncoding_ : encoding;
}

WString operator+ (const WString& lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const std::wstring& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const std::u16string& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const std::u32string& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const wchar_t *rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const char16_t *rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const char32_t *rhs)
{
  WString result = lhs;
  return result += rhs;
}

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

WString operator+ (const std::wstring& lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const std::u16string& lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const std::u32string& lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const wchar_t *lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const char16_t *lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const char32_t *lhs, const WString& rhs)
{
  WString result = lhs;
  return result += rhs;
}

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
  return WString(lhs) == rhs;
}

bool operator== (const std::string& lhs, const WString& rhs)
{
  return WString(lhs) == rhs;
}

bool operator== (const std::wstring& lhs, const WString& rhs)
{
  return WString(lhs) == rhs;
}

bool operator== (const std::u16string& lhs, const WString& rhs)
{
  return WString(lhs) == rhs;
}

bool operator== (const std::u32string& lhs, const WString& rhs)
{
  return WString(lhs) == rhs;
}

bool operator== (const wchar_t *lhs, const WString& rhs)
{
  return WString(lhs) == rhs;
}

bool operator== (const char16_t *lhs, const WString& rhs)
{
  return WString(lhs) == rhs;
}

bool operator== (const char32_t *lhs, const WString& rhs)
{
  return WString(lhs) == rhs;
}

bool operator!= (const char *lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

bool operator!= (const std::string& lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

bool operator!= (const std::wstring& lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

bool operator!= (const std::u16string& lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

bool operator!= (const std::u32string& lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

bool operator!= (const wchar_t *lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

bool operator!= (const char16_t *lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

bool operator!= (const char32_t *lhs, const WString& rhs)
{
  return WString(lhs) != rhs;
}

void WString::makeLiteral()
{
  if (!literal()) {
    utf8_ = resolveKey(TextFormat::Plain);
    impl_->key_ = std::string();
  }
}

std::wostream& operator<< (std::wostream& lhs, const WString& rhs)
{
  return streamUTF8(lhs, rhs.toUTF8());
}

std::ostream& operator<< (std::ostream& lhs, const WString& rhs)
{
  return lhs << rhs.narrow();
}

}

#endif // WT_CNOR

