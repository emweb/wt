/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Http/Message.h"

#include <cstring>

#ifdef WT_WIN32
#define strcasecmp _stricmp
#endif

namespace Wt {
  namespace Http {

Message::Header::Header()
{ }

Message::Header::Header(const std::string& name, const std::string& value)
  : name_(name),
    value_(value)
{ }

Message::Header::Header(const Header& other)
  : name_(other.name_),
    value_(other.value_)
{ }

void Message::Header::setName(const std::string& name)
{
  name_ = name;
}

void Message::Header::setValue(const std::string& value)
{
  value_ = value;
}

Message::Message(std::vector<Header> headers)
  : status_(-1),
    headers_(headers)
{ } 

Message::Message()
  : status_(-1)
{ }

Message::Message(const Message& other)
  : status_(other.status_),
    headers_(other.headers_)
{
  body_ << other.body_.str();
}

void Message::setStatus(int status)
{
  status_ = status;
}

void Message::setHeader(const std::string& name, const std::string& value)
{
  for (unsigned i = 0; i < headers_.size(); ++i) {
    if (headers_[i].name() == name) {
      headers_[i].setValue(value);
      return;
    }
  }

  addHeader(name, value);
}

void Message::addHeader(const std::string& name, const std::string& value)
{
  headers_.push_back(Header(name, value));
}

const std::string *Message::getHeader(const std::string& name) const
{
  for (unsigned i = 0; i < headers_.size(); ++i)
    if (strcasecmp(headers_[i].name().c_str(),name.c_str()) == 0)
      return &headers_[i].value();

  return nullptr;
}

void Message::addBodyText(const std::string& text)
{
  body_ << text;
}

std::string Message::body() const
{
  return body_.str();
}

  }
}
