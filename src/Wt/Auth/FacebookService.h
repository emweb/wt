// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_FACEBOOK_AUTH_H_
#define WT_AUTH_FACEBOOK_AUTH_H_

#include <Wt/Auth/OAuthService.h>

namespace Wt {
  namespace Auth {

/*! \brief %OAuth service for Facebook as third-party authenticator.
 *
 * The configuration of the service is done using properties, whose
 * values need to match the values configured at Facebook.
 *
 * - <tt>facebook-oauth2-redirect-endpoint</tt>: the URL of the local
 *   redirect endpoint, to which the Facebook OAuth service redirects the user
 *   after authentication. See also redirectEndpoint()
 * - <tt>facebook-oauth2-redirect-endpoint-path</tt>: optionally, the
 *   deployment path that corresponds to the redirect endpoint. See
 *   also redirectEndpointPath()
 * - <tt>facebook-oauth2-app-id</tt>: The application ID
 * - <tt>facebook-oauth2-app-secret</tt>: The application secret.
 *
 * For example:
 * \code
 * <properties>
 *   <property name="facebook-oauth2-redirect-endpoint">
 *     http://localhost:8080/oauth2callback
 *   </property>
 *   <property name="facebook-oauth2-app-id">
 *     1234567890123456
 *   </property>
 *   <property name="facebook-oauth2-app-secret">
 *     a3cf1630b1ae415c7260d849efdf444d
 *   </property>
 * </properties>
 * \endcode
 *
 * Like all <b>service classes</b>, this class holds only
 * configuration state. Thus, once configured, it can be safely shared
 * between multiple sessions since its state (the configuration) is
 * read-only. 
 * \if cpp
 * A "const FacebookService" object is thus thread-safe.
 * \endif
 *
 * \if cpp
 * \sa http://developers.facebook.com/docs/authentication/
 * \elseif java
 * See also: http://developers.facebook.com/docs/authentication/
 * \endif
 *
 * \if cpp
 * \note For FastCGI, see also additional configuration in OAuthService
 * \endif
 *
 * \ingroup auth
 */
class WT_API FacebookService : public OAuthService
{
public:
  /*! \brief Constructor.
   */
  FacebookService(const AuthService& baseAuthService);

  /*! \brief Checks whether a FacebookAuth service is properly configured.
   *
   * This returns \c true if a value is found for the three
   * configuration properties.
   */
  static bool configured();

  virtual std::string name() const override;
  virtual WString description() const override;
  virtual int popupWidth() const override;
  virtual int popupHeight() const override;

  virtual std::string authenticationScope() const override;
  virtual std::string redirectEndpoint() const override;
  virtual std::string redirectEndpointPath() const override;
  virtual std::string authorizationEndpoint() const override;
  virtual std::string tokenEndpoint() const override;
  virtual std::string clientId() const override;
  virtual std::string clientSecret() const override;
  virtual ClientSecretMethod clientSecretMethod() const override;

  virtual Http::Method tokenRequestMethod() const override;
  virtual std::unique_ptr<OAuthProcess>
    createProcess(const std::string& scope) const override;
};

  }
}

#endif // WT_AUTH_FACEBOOK_AUTH_H_
