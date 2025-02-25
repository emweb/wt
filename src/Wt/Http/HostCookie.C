/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Http/HostCookie.h"

#include "Wt/WLogger.h"

namespace Wt {
  LOGGER("Http::HostCookie");

  namespace Http {

HostCookie::HostCookie(const std::string& name)
  : Cookie("__Host-"+name, "")
{
  init(name);
}

HostCookie::HostCookie(const std::string& name, const std::string& value)
  : Cookie("__Host-"+name, value)
{
  init(name);
}

#ifndef WT_TARGET_JAVA
HostCookie::HostCookie(const std::string& name, const std::string& value, const Wt::WDateTime& expires)
  : Cookie("__Host-"+name, value, expires)
{
  init(name);
}

HostCookie::HostCookie(const std::string& name, const std::string& value,
                       const std::chrono::seconds& maxAge)
  : Cookie("__Host-"+name, value, maxAge)
{
  init(name);
}
#endif

void HostCookie::setSecure(bool secure)
{
  LOG_WARN("Trying to add/remove the secure flag of a HostCookie. This flag is always set on a HostCookie. This change will be ignored.");
}

void HostCookie::setPath(const std::string& path)
{
  LOG_WARN("Trying to change the path of a HostCookie. The path of a HostCookie must always be '/'. This change will be ignored.");
}

void HostCookie::setDomain(const std::string& domain)
{
  LOG_WARN("Trying to change the domain of a HostCookie. A HostCookie cannot have any domain set. This change will be ignored.");
}

void HostCookie::init(std::string name)
{
  logicName_ = name;
  Cookie::setSecure(true);
  Cookie::setPath("/");
  Cookie::setDomain("");
}

  }
}
