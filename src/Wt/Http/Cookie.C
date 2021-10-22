#include "Wt/Http/Cookie.h"

#include "Wt/Utils.h"
#include "Wt/WApplication.h"
#include "Wt/WDate.h"
#include "Wt/WDateTime.h"
#include "Wt/WEnvironment.h"
#include "Wt/WStringStream.h"

namespace Wt {
  namespace Http {

Cookie::Cookie(const std::string& name)
  : name_(name)
{ }

Cookie::Cookie(const std::string& name, const std::string& value)
  : name_(name),
    value_(value)
{ }

Cookie::Cookie(const std::string& name, const std::string& value, const Wt::WDateTime& expires)
  : name_(name),
    value_(value),
    expires_(expires)
{ }

Cookie::Cookie(const std::string& name, const std::string& value,
               const std::chrono::seconds& maxAge)
  : name_(name),
    value_(value),
    maxAge_(maxAge)
{ }

void Cookie::setValue(const std::string& value)
{
  value_ = value;
}

void Cookie::setExpires(const Wt::WDateTime& expires)
{
  expires_ = expires;
}

void Cookie::setMaxAge(const std::chrono::seconds& maxAge)
{
  maxAge_ = maxAge;
}

void Cookie::setDomain(const std::string& domain)
{
  domain_ = domain;
}

void Cookie::setPath(const std::string& path)
{
  path_ = path;
}

void Cookie::setSecure(bool secure)
{
  secure_ = secure;
}

void Cookie::setHttpOnly(bool httpOnly)
{
  httpOnly_ = httpOnly;
}

void Cookie::setSameSite(SameSite sameSite)
{
  sameSite_ = sameSite;
}

}
}
