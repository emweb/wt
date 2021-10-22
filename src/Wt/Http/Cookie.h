// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_HTTP_COOKIE_H_
#define WT_HTTP_COOKIE_H_

#include <Wt/WDateTime.h>
#include <Wt/WGlobal.h>

#include <string>

namespace Wt {
  namespace Http {

/*! \class Cookie Wt/Http/Cookie.h Wt/Http/Cookie.h
 * \brief An HTTP Cookie
 *
 * Cookies can be set with WApplication::setCookie(const Http::Cookie&) and
 * removed with WApplication::removeCookie(const Http::Cookie&).
 *
 * \sa WApplication::setCookie(const Http::Cookie&), WApplication::removeCookie(const Http::Cookie&), WEnvironment::supportsCookies(), WEnvironment::getCookie()
 *
 * \ingroup http
 */
class WT_API Cookie
{
public:
  /*! \brief Enumeration of SameSite attribute values.
   */
  enum class SameSite {
    None,   //!< Sent in all contexts (note: generally not allowed if secure not specified)
    Lax,    //!< Sent for first-party and top-level GET from third-party
    Strict  //!< Sent for first-party only
  };

  /*! \brief Constructor for cookie without value.
   *
   * Allows you to only specify the name of the cookie. This can be useful if the intention
   * is to pass it to WApplication::removeCookie(), in which case no value is needed.
   *
   * \sa WApplication::removeCookie(const Http::Cookie&)
   */
  explicit Cookie(const std::string& name);

  /*! \internal
   *  \brief Constructor for a session cookie.
   */
  Cookie(const std::string& name, const std::string& value);

  /*! \brief Constructor for a cookie that expires at a certain datetime.
   *
   * The name must be a valid cookie name (of type 'token': no special
   * characters or separators, see RFC2616 page 16). The value may be
   * anything.
   *
   * \sa WApplication::setCookie(const Http::Cookie&)
   */
  Cookie(const std::string& name, const std::string& value,
         const Wt::WDateTime& expires);

  /*! \brief Constructor for a cookie that expires after certain duration.
   *
   * The name must be a valid cookie name (of type 'token': no special
   * characters or separators, see RFC2616 page 16). The value may be
   * anything.
   *
   * \sa WApplication::setCookie(const Http::Cookie&)
   */
  Cookie(const std::string& name, const std::string& value,
         const std::chrono::seconds& maxAge);

  /*! \brief The cookie name.
   */
  const std::string& name() const { return name_; }

  /*! \brief Sets the cookie value.
   *
   * \sa value()
   */
  void setValue(const std::string& value);

  /*! \brief The cookie value.
   *
   * \sa setValue()
   */
  const std::string& value() const { return value_; }

  /*! \brief Sets when the cookie will expire.
   *
   * If WDateTime::isNull(), it will be a session cookie (which expires when the
   * browser is closed).
   *
   * \sa expires(), setMaxAge()
   */
  void setExpires(const Wt::WDateTime& expires);

  /*! \brief The cookie expiration.
   *
   * \sa setExpires(), setMaxAge()
   */
  const Wt::WDateTime& expires() const { return expires_; }

#ifdef WT_TARGET_JAVA
  void setMaxAge(int maxAge);
  int maxAge() const;
#else
  /*! \brief Sets the cookie's Max-Age.
   *
   * The number of seconds until the cookie expires. Note that if both Expires
   * and Max-Age are set, maxAge has precedence.
   * When the duration is negative, Max-Age will not be set.
   *
   * \sa maxAge(), setExpires()
   */
  void setMaxAge(const std::chrono::seconds& maxAge);

  /*! \brief The cookie's Max-Age
   *
   * \sa setMaxAge(), setExpires()
   */
  const std::chrono::seconds& maxAge() const { return maxAge_; }
#endif

  /*! \brief Sets the cookie domain.
   *
   * By default the Domain attribute is omitted, which the browser interprets as
   * the current domain (not including subdomains).
   *
   * \sa domain()
   */
  void setDomain(const std::string& domain);

  /*! \brief The cookie domain.
   *
   * \sa setDomain()
   */
  const std::string& domain() const { return domain_; }

  /*! \brief Sets the cookie path.
   *
   * By default the cookie only applies to the application deployment
   * path (WEnvironment::deploymentPath()).
   *
   * \sa path()
   */
  void setPath(const std::string& path);

  /*! \brief The cookie path.
   *
   * \sa setPath()
   */
  const std::string& path() const { return path_; }

  /*! \brief Sets if the cookie must be sent over a secure connection.
   *
   * The default value is false.
   *
   * \sa secure()
   */
  void setSecure(bool secure);

  /*! \brief Returns if the cookie must be sent over secure connection.
   *
   * \sa setSecure()
   */
  bool secure() const { return secure_; }

  /*! \brief Sets if the cookie is only accessible through HTTP.
   *
   * The default value is true.
   *
   * \sa httpOnly()
   */
  void setHttpOnly(bool httpOnly);

  /*! \brief Returns if the cookie is only accessible through HTTP.
   *
   * \sa setHttpOnly()
   */
  bool httpOnly() const { return httpOnly_; }

  /*! \brief Sets the cookie SameSite attribute.
   *
   * The default value is SameSite::Lax.
   *
   * \note A browser will reject a cookie with SameSite::None if secure() is false.
   *
   * \sa sameSite()
   */
  void setSameSite(SameSite sameSite);

  /*! \brief The cookie SameSite attribute.
   *
   * \sa setSameSite()
   */
  SameSite sameSite() const { return sameSite_; }

private:
  std::string name_;
  std::string value_;
  Wt::WDateTime expires_;
  std::chrono::seconds maxAge_ = std::chrono::seconds(-1);
  std::string domain_;
  std::string path_;
  bool secure_ = false;
  bool httpOnly_ = true;
  SameSite sameSite_ = SameSite::Lax;
};

  }
}

#endif
