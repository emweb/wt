/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Http/SecureCookie.h"

#include "Wt/Utils.h"
#include "Wt/WApplication.h"
#include "Wt/WDate.h"
#include "Wt/WDateTime.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"

namespace Wt {
  LOGGER("Http::SecureCookie");

  namespace Http {

SecureCookie::SecureCookie(const std::string& name)
  : Cookie("__Secure-"+name, "")
{
  init(name);
}

SecureCookie::SecureCookie(const std::string& name, const std::string& value)
  : Cookie("__Secure-"+name, value)
{
  init(name);
}

#ifndef WT_TARGET_JAVA
SecureCookie::SecureCookie(const std::string& name, const std::string& value, const Wt::WDateTime& expires)
  : Cookie("__Secure-"+name, value, expires)
{
  init(name);
}

SecureCookie::SecureCookie(const std::string& name, const std::string& value,
                           const std::chrono::seconds& maxAge)
  : Cookie("__Secure-"+name, value, maxAge)
{
  init(name);
}
#endif

void SecureCookie::setSecure(bool secure)
{
  LOG_WARN("Trying to add/remove the secure flag of a SecureCookie. This flag is always set on a SecureCookie. This change will be ignored.");
}

void SecureCookie::init(std::string name)
{
  logicName_ = name;
  Cookie::setSecure(true);
}

  }
}
