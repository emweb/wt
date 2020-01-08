// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_OAUTH_TOKEN_ENDPOINT_H_
#define WT_OAUTH_TOKEN_ENDPOINT_H_

#include <string>
#include <Wt/WResource.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Auth/User.h>

#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
struct rsa_st;
typedef struct rsa_st RSA;
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA

namespace Wt {
namespace Auth {

/*! \brief Endpoint to retrieve an access token.
 *
 * The token endpoint is used by the client to obtain an
 * OAuthAccessToken by presenting its authorization grant.
 * This implementation only supports the "authorization_code"
 * grant type.  The client ID and secret can be passed with Basic auth
 * or by POST request parameters.  When something goes wrong, the
 * reply will include a JSON object with an "error" attribute.
 *
 * This endpoint is implemented as a WResource, so it's usually
 * deployed using WServer::addResource.
 *
 * For more information refer to the specification:
 * https://tools.ietf.org/rfc/rfc6749.txt
 *
 * When the scope includes "openid" an ID Token will be included as
 * specified by the OpenID Connect standard.
 *
 * This class relies on a correct implementation of several function
 * in the AbstractUserDatabase. Namely
 * AbstractUserDatabase::idpClientFindWithId,
 * AbstractUserDatabase::idpClientAuthMethod,
 * AbstractUserDatabase::idpVerifySecret,
 * AbstractUserDatabase::idpClientId,
 * AbstractUserDatabase::idpTokenFindWithValue,
 * AbstractUserDatabase::idpTokenAdd,
 * AbstractUserDatabase::idpTokenRemove,
 * AbstractUserDatabase::idpTokenRedirectUri,
 * AbstractUserDatabase::idpTokenAuthClient,
 * AbstractUserDatabase::idpTokenUser, and
 * AbstractUserDatabase::idpTokenScope.
 *
 * Must be deployed using TLS.
 */
class WT_API OAuthTokenEndpoint : public WResource
{
public:
  /*! \brief Constructor.
   *
   * The issuer argument is used for the "iss" attribute in the ID
   * Token when the scope includes "openid".
   */
  OAuthTokenEndpoint(AbstractUserDatabase& db,
                     std::string issuer);

  ~OAuthTokenEndpoint();

  virtual void handleRequest(const Http::Request& request, Http::Response& response) override;

  /*! \brief Sets the amount of seconds after which generated access
   * tokens expire.
   *
   * Defaults to 3600 seconds.
   *
   */
  void setAccessExpSecs(int seconds);

  /*! \brief Sets the amount of seconds after which generated id
   * tokens expire.
   *
   * Defaults to 3600 seconds.
   *
   */
  void setIdExpSecs(int seconds);

#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
  // incomplete feature
  void setRSAKey(const std::string &path);
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA
private:
  /*! \brief Is only called when scope contains openid. Generates a JSON Web Token.
     */
  virtual const std::string idTokenPayload(const std::string &clientId, const std::string& scope, const User& user);

  AbstractUserDatabase *db_;
  int accessExpSecs_;
  int idExpSecs_;
  std::string iss_;
  static std::string methodToString(ClientSecretMethod method);

#ifndef WT_TARGET_JAVA
#ifdef WT_WITH_SSL
  RSA *privateKey;
  std::string rs256(const std::string &input);
#endif // WT_WITH_SSL
#endif // WT_TARGET_JAVA
};

}
}

#endif // WT_OAUTH_TOKEN_ENDPOINT_H_
