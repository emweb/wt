// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_ISSUED_TOKEN_H_
#define WT_AUTH_ISSUED_TOKEN_H_

#include <string>
#include "Wt/WDateTime.h"
#include "Wt/Auth/User.h"
#include "Wt/Auth/OAuthClient.h"

namespace Wt {
namespace Auth {

class OAuthClient;

/*! \brief Token or authorization code that was issued to a relying party.
 *
 * This class represents an access token. It is a value class that stores only
 * the id and a reference to an AbstractUserDatabase to access its properties.
 *
 * An object can point to a valid token, or be invalid. Invalid tokens are
 * typically used as return value for database queries which did not match with
 * an existing client.
 *
 * \sa AbstractUserDatabase
 *
 */
class WT_API IssuedToken
{
public:
  /*! \brief Default constructor.
   *
   * Creates an invalid token.
   *
   * \sa checkValid()
   */
  IssuedToken();

  /*! \brief Constructor.
   *
   * Creates a user with id \p id, and whose information is stored in
   * the \p database.
   */
  IssuedToken(const std::string& id, const AbstractUserDatabase& userDatabase);

  /*! \brief Returns whether the token is valid.
   *
   * A invalid token is a sentinel value returned by methods that query
   * the database but could not identify a matching user.
   */
  bool checkValid() const;

  /*! \brief Returns the user id.
   *
   * This returns the id that uniquely identifies the token, and acts
   * as a "primary key" to obtain other information for the token in
   * the database.
   *
   * \sa AbstractUserDatabase
   */
  const std::string id() const { return id_; }

  /*! \brief Retrieves the string value that represents this token,
   * usually random characters.
   */
  const std::string value() const;

  /*! \brief Retrieves the time when the token expires.
   */
  const WDateTime expirationTime() const;

  /*! \brief Retrieves the purpose of this token: authenication code,
   * access token or refresh token.
   */
  const std::string purpose() const;

  /*! \brief Retrieves the scope of this token as a space-separated
   * string.
   */
  const std::string scope() const;

  /*! \brief Retrieves the valid redirect uri of this token.
   */
  const std::string redirectUri() const;

  /*! \brief Retrieves the user that is associated with this token.
   */
  const User user() const;

  /*! \brief Retrieves the client for which this token was issued.
   */
  const OAuthClient authClient() const;
private:
  std::string id_;
  AbstractUserDatabase *db_;
};

}
}

#endif // WT_AUTH_ISSUED_TOKEN_H_
