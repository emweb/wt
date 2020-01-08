// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_OAUTHCLIENT_H_
#define WT_AUTH_OAUTHCLIENT_H_

#include <set>
#include <string>

#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Auth/WAuthGlobal.h>

namespace Wt {
namespace Auth {

class AbstractUserDatabase;

/*! \class OAuthClient Wt/Auth/OAuthClient.h \brief OAuth 2.0 client
 * implementing OpenID Connect, a.k.a. relying party.
 *
 * This class represents a client. It is a value class that stores
 * only the id and a reference to an AbstractUserDatabase to access
 * its properties.
 *
 * An object can point to a valid client, or be invalid. Invalid
 * clients are typically used as return value for database queries
 * which did not match with an existing client.
 *
 * \sa AbstractUserDatabase
 *
 */
class WT_API OAuthClient
{
public:

  /*! \brief Default constructor that creates an invalid OAuthClient.
   *
   * \sa checkValid()
   */
  OAuthClient();

  /*! \brief Constructor
   *
   * Creates a client with id \p id, and whose information is stored in
   * the \p database.
   */
  OAuthClient(const std::string& id, const AbstractUserDatabase& db);

  /*! \brief Returns whether the user is valid.
   *
   * A invalid user is a sentinel value returned by methods that query
   * the database but could not identify a matching user.
   */
  bool checkValid() const;

  /*! \brief Returns the ID used to identify the client in the database.
   *
   * This returns the id that uniquely identifies the user, and acts
   * as a "primary key" to obtain other information for the user in
   * the database.
   */
  std::string id() const;

  /*! \brief Returns the ID used to identify the client with the
   * OpenID Connect provider and user.
   *
   * This is the id that the client uses to identify itself with the
   * identity provider.
   *
   * \sa AbstractUserDatabase::idpClientId()
   */
  std::string clientId() const;

  /*! \brief Returns true if the given secret is correct for the given
   * client.
   *
   * \sa AbstractUserDatabase::verifySecret()
   */
  bool verifySecret(const std::string& secret) const;

  /*! \brief Returns the set of redirect URI's that are valid for this
   * client.
   *
   * \sa AbstractUserDatabase::idpClientRedirectUris()
   */
  std::set<std::string> redirectUris() const;

  /*! \brief Returns whether the client is confidential or public.
   *
   * \sa AbstractUserDatabase::idpClientConfidential()
   */
  bool confidential() const;

  /*! \brief Returns the client authentication method (see OIDC Core
   * chapter 9)
   *
   * \sa AbstractUserDatabase::idpClientAuthMethod()
   */
  ClientSecretMethod authMethod() const;
  //+ maak er een trait class bij

private:
  const AbstractUserDatabase *db_;
  std::string id_;
};

}
}

#endif // WT_AUTH_OAUTHCLIENT_H_
