// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HTTP_HOST_COOKIE_H_
#define WT_HTTP_HOST_COOKIE_H_

#include <Wt/WDateTime.h>
#include <Wt/WGlobal.h>
#include <Wt/Http/Cookie.h>

#include <string>

namespace Wt {
  namespace Http {

/*! \class HostCookie Wt/Http/HostCookie.h Wt/Http/HostCookie.h
 * \brief An HTTP Cookie with the "__Host-" prefix.
 * 
 * This is a helper class used to create a cookie with the "__Host-"
 * prefix.
 * 
 * A Cookie with the "__Host-" prefix can only be set over a HTTPS
 * connection, and can only be overwritten by the domain that set it.
 * It also enforces the Domain attribute of the cookie to be empty, the
 * secure flag be set, and the Path attribute to be set to "/".
 * 
 * This protects the cookie from being altered by a compromised
 * subdomain, stopping a potential attack vector.
 * 
 * \sa WEnvironment::getHostCookie(), 
 *     WApplication::setCookie(const Http::Cookie&),
 *     WApplication::removeCookie(const Http::Cookie&),
 *     WEnvironment::supportsCookies(),
 *     SecureCookie
 * 
 * \ingroup http
 */
class WT_API HostCookie: public Cookie
{
public:
  /*! \brief Constructor for a host cookie without value.
   *
   * Allows you to only specify the name of the cookie. This can be useful if the intention
   * is to pass it to WApplication::removeCookie(), in which case no value is needed.
   *
   * \sa WApplication::removeCookie(const Http::Cookie&)
   * 
   * \note The prefix is added automatically to the name.
   */
  explicit HostCookie(const std::string& name);

  /*! \internal
   *  \brief Constructor for a session host cookie.
   */
  HostCookie(const std::string& name, const std::string& value);

#ifndef WT_TARGET_JAVA
  /*! \brief Constructor for a host cookie that expires at a certain datetime.
   *
   * The name must be a valid cookie name (of type 'token': no special
   * characters or separators, see RFC2616 page 16). The value may be
   * anything.
   *
   * \sa WApplication::setCookie(const Http::Cookie&)
   * 
   * \note The prefix is added automatically to the name.
   */
  HostCookie(const std::string& name, const std::string& value,
             const Wt::WDateTime& expires);
  
  /*! \brief Constructor for a host cookie that expires after certain duration.
   *
   * The name must be a valid cookie name (of type 'token': no special
   * characters or separators, see RFC2616 page 16). The value may be
   * anything.
   *
   * \sa WApplication::setCookie(const Http::Cookie&)
   * 
   * \note The prefix is added automatically to the name.
   */
  HostCookie(const std::string& name, const std::string& value,
             const std::chrono::seconds& maxAge);
#endif
  
  /*! \brief Returns name of the cookie without the prefix.
   */
  std::string logicName() const { return logicName_; }
  
  /*! \brief Logs a warning.
   * 
   * This function logs a warning instead of enabling/disabling the
   * secure flag, since the secure flag must always be set on a cookie
   * with the "__Host-" prefix.
   */
  void setSecure(bool secure) override;

  /*! \brief Logs a warning.
   * 
   * This function logs a warning instead of changing the path, since
   * the path of a cookie with the "__Host-" prefix must always be set
   * to "/".
   */
  void setPath(const std::string& path) override;

  /*! \brief Logs a warning.
   * 
   * This function logs a warning instead of changing the domain, since
   * the domain of a cookie with the "__Host-" prefix must always be
   * empty.
   */
  void setDomain(const std::string& domain) override;

private:
  std::string logicName_;

  void init(std::string name);
};

  }
}

#endif
