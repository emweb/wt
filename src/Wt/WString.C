/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WString"
#include "Wt/WStringUtil"
#include "Wt/WWebWidget"

#include "WtException.h"
#include "Utils.h"

namespace Wt {

std::vector<std::string> WString::stArguments_;
const WString WString::emptyString;

WString::WString()
  : impl_(0)
{ }

WString::WString(const wchar_t *value)
  : impl_(0)
{ 
  if (value)
    utf8_ = Wt::toUTF8(value);
}

WString::WString(const std::wstring& value)
  : impl_(0)
{ 
  utf8_ = Wt::toUTF8(value);
}

WString::WString(const char *value, CharEncoding encoding)
  : impl_(0)
{
  if (value)
    if (encoding == UTF8)
      utf8_ = value;
    else
      utf8_ = Wt::toUTF8(widen(value));
}

WString::WString(const std::string& value, CharEncoding encoding)
  : impl_(0)
{ 
  if (encoding == UTF8)
    utf8_ = value;
  else
    utf8_ = Wt::toUTF8(widen(value));
}

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
  return value() < rhs.value();
}

bool WString::operator> (const WString& rhs) const
{
  return value() > rhs.value();
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

WString& WString::operator+= (const std::wstring& rhs)
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

WString& WString::operator+= (const std::string& rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(Wt::widen(rhs));

  return *this;
}

WString& WString::operator+= (const char *rhs)
{
  makeLiteral();
  utf8_ += Wt::toUTF8(Wt::widen(rhs));

  return *this;
}

bool WString::empty() const
{
  if (literal())
    return utf8_.empty();
  else
    return toUTF8().empty();
}

WString WString::fromUTF8(const std::string& value)
{
  return WString(value, UTF8);
}

WString WString::fromUTF8(const char *value)
{
  return WString(value, UTF8);
}

std::string WString::toUTF8() const
{
  if (impl_) {
    std::string result = utf8_;

    if (!impl_->key_.empty())
      wApp->messageResourceBundle().resolveKey(impl_->key_, result);

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

std::wstring WString::value() const
{
  return Wt::fromUTF8(toUTF8());
}

std::string WString::narrow() const
{
  return Wt::narrow(value());
}

WString::operator std::wstring() const
{
  return value();
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

  if (encoding == UTF8)
    impl_->arguments_.push_back(value);
  else
    impl_->arguments_.push_back(Wt::toUTF8(Wt::widen(value)));

  return *this;
}

WString& WString::arg(const char *value, CharEncoding encoding)
{
  return arg(std::string(value), encoding);
}

WString& WString::arg(const std::wstring& value)
{
  createImpl();

  impl_->arguments_.push_back(Wt::toUTF8(value));

  return *this;
}

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

WString operator+ (const WString& lhs, const std::wstring& rhs)
{
  WString result = lhs;
  return result += rhs;
}

WString operator+ (const WString& lhs, const wchar_t *rhs)
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

WString operator+ (const wchar_t *lhs, const WString& rhs)
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
  return rhs == lhs;
}

bool operator== (const std::string& lhs, const WString& rhs)
{
  return rhs == lhs;
}

bool operator== (const std::wstring& lhs, const WString& rhs)
{
  return rhs == lhs;
}

bool operator== (const wchar_t *lhs, const WString& rhs)
{
  return rhs == lhs;
}


bool operator!= (const char *lhs, const WString& rhs)
{
  return !(rhs == lhs);
}

bool operator!= (const std::string& lhs, const WString& rhs)
{
  return !(rhs == lhs);
}

bool operator!= (const std::wstring& lhs, const WString& rhs)
{
  return !(rhs == lhs);
}

bool operator!= (const wchar_t *lhs, const WString& rhs)
{
  return !(rhs == lhs);
}

void WString::makeLiteral()
{
  if (!literal()) {
    wApp->messageResourceBundle().resolveKey(impl_->key_, utf8_);
    impl_->key_ = std::string();
  }
}

std::wostream& operator<< (std::wostream& lhs, const WString& rhs)
{
  return lhs << rhs.value();
}

std::ostream& operator<< (std::ostream& lhs, const WString& rhs)
{
  return lhs << rhs.narrow();
}

}
