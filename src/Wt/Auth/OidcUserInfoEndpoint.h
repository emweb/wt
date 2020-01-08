// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_OIDC_USER_INFO_ENDPOINT_H_
#define WT_AUTH_OIDC_USER_INFO_ENDPOINT_H_

#include <Wt/WResource.h>

#include <Wt/Auth/User.h>
#include <Wt/Auth/AbstractUserDatabase.h>

#include <map>

namespace Wt {
namespace Auth {

/*! \class OidcUserInfoEndpoint Wt/Auth/OidcUserInfoEndpoint.h
 *  \brief Endpoint at which user info can be requested.
 *
 * The UserInfo Endpoint is an OAuth 2.0 Protected Resource that
 * returns Claims about the authenticated End-User. To obtain the
 * requested Claims about the End-User, the Client makes a request to
 * the UserInfo Endpoint using an Access Token obtained through
 * OpenID Connect Authentication. These Claims are normally
 * represented by a JSON object that contains a collection of name
 * and value pairs for the Claims.
 *
 * One can use setScopeToken to map claims to a scopeToken. The value
 * of these claims will be retrieved using the
 * AbstractUserDatabase::idpJsonClaim function.
 *
 * You can look at
 * http://openid.net/specs/openid-connect-core-1_0.html#UserInfo for
 * more information.
 *
 * This endpoint is implemented as a WResource, so it's usually
 * deployed using WServer::addResource.
 *
 * This class relies on the implementation of several functions in the
 * AbstractUserDatabase. Namely AbstractUserDatabase::idpJsonClaim,
 * AbstractUserDatabase::idpTokenFindWithValue,
 * AbstractUserDatabase::idpTokenUser, and
 * AbstractUserDatabase::idpTokenScope.
 *
 * Must be deployed using TLS.
 *
 * \sa setScopeToken
 * \sa AbstractUserDatabase
 */
class WT_API OidcUserInfoEndpoint : public WResource
{
public:
  /*! \brief Constructor.
   */
  OidcUserInfoEndpoint(AbstractUserDatabase& db);

  virtual ~OidcUserInfoEndpoint();

  virtual void handleRequest(const Http::Request& request,
                             Http::Response& response) override;


  /*! \brief Maps the given scope token to the given set of claims.
   *
   * The value of these claims will be retrieved from the
   * AbstractUserDatabase using the AbstractUserDatabase::idpJsonClaim function.
   *
   * At construction, the following default scopes are automatically
   * populated: profile -> {name} and email -> {email, email_verified}
   *
   * A scope can be erased by setting it to an empty set of claims.
   *
   * \sa AbstractUserDatabase::idpJsonClaim
  */
  void setScopeToken(const std::string& scopeToken,
                     const std::set<std::string>& claims);

  /*! \brief Retrieves the set of claims that has been mapped to the
   * given scope token.
  */
  const std::map<std::string,std::set<std::string> > &scopeTokens() const;

protected:

  /*! \brief Generates the JSON containing the claims for the given
   * scope.
   *
   * Can be overridden, but by default it uses the configured
   * mapping set by setScopeToken, and AbstractUserDatabase::idpJsonClaim.
   *
   * \sa AbstractUserDatabase::idpJsonClaim
  */
  virtual Json::Object generateUserInfo(const User& user,
                                        const std::set<std::string>& scope);
private:
  AbstractUserDatabase *db_;
  std::map<std::string,std::set<std::string> > claimMap_;
};

}
}

#endif // WT_AUTH_OIDC_USER_INFO_ENDPOINT_H_
