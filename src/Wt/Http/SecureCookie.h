// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HTTP_SECURE_COOKIE_H_
#define WT_HTTP_SECURE_COOKIE_H_

#include <Wt/WDateTime.h>
#include <Wt/WGlobal.h>
#include <Wt/Http/Cookie.h>

#include <string>

namespace Wt {
  namespace Http {

/*! \class SecureCookie Wt/Http/SecureCookie.h Wt/Http/SecureCookie.h
 * \brief An HTTP Cookie with the "__Secure-" prefix.
 * 
 * This is a helper class used to create a cookie with the "__Secure-"
 * prefix.
 * 
 * A Cookie with the "__Secure-" prefix must be set by a connection
 * over HTTPS. It also enforces the secure flag to be set.
 * 
 * This protects the cookie from being altered by an unsafe connection,
 * or to have its secure flag removed by a compromised subdomain.
 * 
 * \sa WEnvironment::getSecureCookie(),
 *     WApplication::setCookie(const Http::Cookie&),
 *     WApplication::removeCookie(const Http::Cookie&),
 *     WEnvironment::supportsCookies(),
 *     HostCookie
 * 
 * \ingroup http
 */
class WT_API SecureCookie: public Cookie
{
public:
  /*! \brief Constructor for a secure cookie without value.
   *
   * Allows you to only specify the name of the cookie. This can be useful if the intention
   * is to pass it to WApplication::removeCookie(), in which case no value is needed.
   *
   * \sa WApplication::removeCookie(const Http::Cookie&)
   * 
   * \note The prefix is added automatically to the name.
   */
  explicit SecureCookie(const std::string& name);

  /*! \internal
   *  \brief Constructor for a session secure cookie.
   */
  SecureCookie(const std::string& name, const std::string& value);

#ifndef WT_TARGET_JAVA
  /*! \brief Constructor for a secure cookie that expires at a certain datetime.
   *
   * The name must be a valid cookie name (of type 'token': no special
   * characters or separators, see RFC2616 page 16). The value may be
   * anything.
   *
   * \sa WApplication::setCookie(const Http::Cookie&)
   * 
   * \note The prefix is added automatically to the name.
   */
  SecureCookie(const std::string& name, const std::string& value,
               const Wt::WDateTime& expires);
  
  /*! \brief Constructor for a secure cookie that expires after certain duration.
   *
   * The name must be a valid cookie name (of type 'token': no special
   * characters or separators, see RFC2616 page 16). The value may be
   * anything.
   *
   * \sa WApplication::setCookie(const Http::Cookie&)
   * 
   * \note The prefix is added automatically to the name.
   */
  SecureCookie(const std::string& name, const std::string& value,
               const std::chrono::seconds& maxAge);
#endif
  
  /*! \brief Returns name of the cookie without the prefix.
   */
  std::string logicName() const { return logicName_; }
  
  /*! \brief Logs a warning.
   * 
   * This function logs a warning instead of enabling/disabling the
   * secure flag, since the secure flag must always be set on a cookie
   * with the "__Secure-" prefix.
   */
  void setSecure(bool secure) override;

private:
  std::string logicName_;

  void init(std::string name);
};

  }
}

#endif
