// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef GOOGLE_SERVICE_H_
#define GOOGLE_SERVICE_H_

#include <Wt/Auth/OidcService.h>

namespace Wt {
  namespace Auth {

/*! \brief %OAuth service for Google as third-party authenticator.
 *
 * The configuration of the service is done using properties, whose
 * values need to match the values configured at Google.
 *
 * - <tt>google-oauth2-redirect-endpoint</tt>: the URL of the local
 *   redirect endpoint, to which the google OAuth service redirects the user
 *   after authentication. See also redirectEndpoint()
 * - <tt>google-oauth2-redirect-endpoint-path</tt>: optionally, the deployment
 *   path that corresponds to the redirect endpoint. See also
 *   redirectEndpointPath()
 * - <tt>google-oauth2-client-id</tt>: The client ID
 * - <tt>google-oauth2-client-secret</tt>: The client secret.
 *
 * For example:
 * \code
 * <properties>
 *   <property name="google-oauth2-redirect-endpoint">
 *     http://localhost:8080/oauth2callback
 *   </property>
 *   <property name="google-oauth2-client-id">
 *     123456789012.apps.googleusercontent.com
 *   </property>
 *   <property name="google-oauth2-client-secret">
 *     abcdefghijk-12312312312
 *   </property>
 * </properties>
 * \endcode
 *
 * Like all <b>service classes</b>, this class holds only
 * configuration state. Thus, once configured, it can be safely shared
 * between multiple sessions since its state (the configuration) is
 * read-only.
 * \if cpp
 * A "const GoogleService" object is thus thread-safe.
 * \endif
 *
 * \if cpp
 * \sa https://developers.google.com/identity/protocols/OAuth2
 * \sa https://developers.google.com/identity/protocols/OpenIDConnect
 * \elseif java
 * See also: https://developers.google.com/identity/protocols/OAuth2
 *           https://developers.google.com/identity/protocols/OpenIDConnect
 * \endif
 *
 * \if cpp
 * \note For FastCGI, see also additional configuration in OAuthService
 * \endif
 *
 * \ingroup auth
 */
class WT_API GoogleService : public OidcService
{
public:
  /*! \brief Constructor.
   */
  GoogleService(const AuthService& baseAuth);

  /*! \brief Checks whether a GoogleAuth service is properly configured.
   *
   * This returns \c true if a value is found for the three
   * configuration properties.
   */
  static bool configured();

  virtual std::string redirectEndpointPath() const override;
};

  }
}

#endif // GOOGLE_SERVICE_H_
