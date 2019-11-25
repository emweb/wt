// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_ABSTRACT_USER_DATABASE_H_
#define WT_AUTH_ABSTRACT_USER_DATABASE_H_

#include <Wt/Auth/User.h>
#include <Wt/Auth/IssuedToken.h>
#include <Wt/Auth/OAuthClient.h>
#include <Wt/Auth/WAuthGlobal.h>
#include <Wt/Json/Value.h>

#include <set>

namespace Wt {
  namespace Auth {

class IssuedToken;
class OAuthClient;

/*! \class AbstractUserDatabase Wt/Auth/AbstractUserDatabase.h
 *  \brief Abstract interface for an authentication user database
 *
 * This class defines the interface for managing user data related to
 * authentication. You need to implement this interface to allow the
 * authentication service classes (AuthService, PasswordService,
 * OAuthService, and OidcService) to locate and update user
 * credentials. Except for functions which do work on a single user,
 * it is more convenient to use the User API.  Obviously, you may have
 * more data associated with a user, including roles for access
 * control, other personal information, address information. This
 * information cannot be accessed through the Auth::User class, but
 * you should make it available through your own User class, which is
 * then als the basis of this user database implementation.
 *
 * The only assumption made by the authentication system is that an id
 * uniquely defines the user. This is usually an internal identifier,
 * for example an auto-incrementing primary key.
 *
 * With a user, one or more other identities may be associated. These
 * could be a login name (for password-based authentication), or id's
 * used by third party providers (such as OAuth or LDAP).
 *
 * The database implements a simple data store and does not contain
 * any logic. The database can store data for different aspects of
 * authentication, but most data fields are only relevant for optional
 * functionality, and thus themeselves optional. The default
 * implementation of these methods will log errors.
 *
 * The authentication views and model classes assume a private
 * instance of the database for each different session, and will try
 * to wrap database access within a transaction. Transaction support
 * can thus be optionally provided by a database implementation.
 *
 * This class is also used by OAuthAuthorizationEndpoint,
 * OAuthTokenEndpoint, and OidcUserInfoEndpoint when implementing an
 * OAuth/OpenID Connect provider to retrieve information not only
 * about the User, but also the OAuthClient, and an IssuedToken.
 *
 * \sa User
 *
 * \ingroup auth
 */
class WT_API AbstractUserDatabase
{
public:
  /*! \brief An abstract transaction.
   *
   * An abstract transaction interface.
   *
   * \sa startTransaction()
   */
  class WT_API Transaction
  {
  public:
    /*! \brief Destructor.
     *
     * If the transaction is not yet finished (committed or rolled back), 
     * it will be rolled back.
     *
     * \sa rollback()
     */
    virtual ~Transaction() noexcept(false);

    /*! \brief Commits the transaction.
     *
     * \sa rollback()
     */
    virtual void commit() = 0;

    /*! \brief Rolls back the transaction.
     *
     * \sa commit()
     */
    virtual void rollback() = 0;
  };

  /*! \brief Destructor.
   */
  virtual ~AbstractUserDatabase();

  /*! \brief Creates a new database transaction.
   *
   * If the underlying database does not support transactions, you can
   * return \c nullptr.
   *
   * Ownership of the transaction is transferred, and the transaction must
   * be deleted after it has been committed or rolled back.
   *
   * The default implementation returns \c nullptr (no transaction support).
   */
  virtual Transaction *startTransaction();

  /*! \brief Finds a user with a given id.
   *
   * The id uniquely identifies a user.
   *
   * This should find the user with the given \p id, or return
   * an invalid user if no user with that id exists.
   */
  virtual User findWithId(const std::string& id) const = 0;

  /*! \brief Finds a user with a given identity.
   *
   * The \p identity uniquely identifies the user by the \p provider.
   *
   * This should find the user with the given \p identity, or return
   * an invalid user if no user with that identity exists.
   */
  virtual User findWithIdentity(const std::string& provider,
				const WT_USTRING& identity) const = 0;

  /*! \brief Adds an identify for the user.
   *
   * This adds an identity to the user.
   *
   * You are free to support only one identity per user, e.g. if you
   * only use password-based authentication. But you may also want to
   * support more than one if you allow the user to login using
   * multiple methods (e.g. name/password, OAuth from one or more
   * providers, LDAP, ...).
   */
  virtual void addIdentity(const User& user, const std::string& provider,
			   const WT_USTRING& id) = 0;

  /*! \brief Changes an identity for a user.
   *
   * The base implementation calls removeIdentity() followed by addIdentity().
   */
  virtual void setIdentity(const User& user, const std::string& provider,
			   const WT_USTRING& id);

  /*! \brief Returns a user identity.
   *
   * Returns a user identity for the given provider, or an empty string
   * if the user has no identitfy set for this provider.
   *
   * \sa addIdentity()
   */
  virtual WT_USTRING identity(const User& user, const std::string& provider)
    const = 0;

  /*! \brief Removes a user identity.
   *
   * This removes all identities of a \p provider from the \p user.
   *
   * \sa addIdentity()
   */
  virtual void removeIdentity(const User& user, const std::string& provider)
    = 0;
  //!@}

  /*! \brief Registers a new user.
   *
   * This adds a new user.
   *
   * This method is only used by view classes involved with
   * registration (RegistrationWidget).
   */
  virtual User registerNew(); 

  /*! \brief Delete a user.
   *
   * This deletes a user from the database.
   */
  virtual void deleteUser(const User& user);

  /*! \brief Returns the status for a user.
   *
   * If there is support for suspending accounts, then this method may
   * be implemented to return whether a user account is disabled.
   *
   * The default implementation always returns AccountStatus::Normal.
   *
   * \sa Login::loginState()
   */
  virtual AccountStatus status(const User& user) const;

  /*! \brief Sets the user status.
   *
   * This sets the status for a user (if supported).
   */
  virtual void setStatus(const User& user, AccountStatus status);

  /** @name Password authentication
   */
  //!@{
  /*! \brief Sets a new user password.
   *
   * This updates the password for a user.
   *
   * This is used only by PasswordService.
   */
  virtual void setPassword(const User& user,
			   const PasswordHash& password);

  /*! \brief Returns a user password.
   *
   * This returns the stored password for a user, or a default
   * constructed password hash if the user does not yet have password
   * credentials.
   *
   * This is used only by PasswordService.
   */
  virtual PasswordHash password(const User& user) const;
  //!@}
  
  /** @name Email addresses (for verification and lost passwords)
   */
  //!@{
  /*! \brief Sets a user's email address.
   *
   * This is used only when email verification is enabled, or as a
   * result of a 3rd party Identity Provider based registration
   * process, if the provider also provides email address information
   * with the identiy.
   *
   * Returns whether the user's email address could be set. This may
   * fail when there is already a user registered that email address.
   *
   * \sa findWithEmail()
   */
  virtual bool setEmail(const User& user, const std::string& address);

  /*! \brief Returns a user's email address.
   *
   * This may be an unverified or verified email address, depending on
   * whether email address verification is enabled in the model
   * classes.
   *
   * This is an optional method, and currently not used by any of the
   * included models or views.
   */
  virtual std::string email(const User& user) const;

  /*! \brief Sets a user's unverified email address.
   *
   * This is only used when email verification is enabled. It holds
   * the currently unverified email address, while a mail is being sent
   * for the user to confirm this email address.
   */
  virtual void setUnverifiedEmail(const User& user,
				  const std::string& address);

  /*! \brief Returns a user's unverified email address.
   *
   * This is an optional method, and currently not used by any of the
   * included models or views.
   */
  virtual std::string unverifiedEmail(const User& user) const;

  /*! \brief Finds a user with a given email address.
   *
   * This is used to verify that a email addresses are unique, and to
   * implement lost password functionality.
   */
  virtual User findWithEmail(const std::string& address) const;

  /*! \brief Sets a new email token for a user.
   *
   * This is only used when email verification is enabled or for lost
   * password functionality.
   */
  virtual void setEmailToken(const User& user, const Token& token,
			     EmailTokenRole role);

  /*! \brief Returns an email token.
   *
   * This is only used when email verification is enabled and for lost
   * password functionality. It should return the email token previously
   * set with setEmailToken()
   */
  virtual Token emailToken(const User& user) const;

  /*! \brief Returns the role of the current email token.
   *
   * This is only used when email verification is enabled or for lost
   * password functionality. It should return the role previously set
   * with setEailToken().
   */
  virtual EmailTokenRole emailTokenRole(const User& user) const;

  /*! \brief Finds a user with a given email token.
   *
   * This is only used when email verification is enabled or for lost
   * password functionality.
   */
  virtual User findWithEmailToken(const std::string& hash) const;
    //!@}

  /** @name Auth tokens (remember-me support)
   */
  //!@{
  /*! \brief Adds an authentication token to a user.
   *
   * Unless you want a user to only have remember-me support from a
   * single computer at a time, you should support multiple
   * authentication tokens per user.
   */
  virtual void addAuthToken(const User& user, const Token& token);

  /*! \brief Deletes an authentication token.
   *
   * Deletes an authentication token previously added with addAuthToken()
   */
  virtual void removeAuthToken(const User& user, const std::string& hash);

  /*! \brief Finds a user with an authentication token.
   *
   * Returns a user with an authentication token.
   *
   * This should find the user associated with a particular token
   * hash, or return an invalid user if no user with that token hash
   * exists.
   */
  virtual User findWithAuthToken(const std::string& hash) const;

  /*! \brief Updates the authentication token with a new hash.
   *
   * If successful, returns the validity of the updated token in seconds.
   *
   * Returns 0 if the token could not be updated because it wasn't found
   * or is expired.
   *
   * Returns -1 if not implemented.
   */
  virtual int updateAuthToken(const User& user, const std::string& oldhash,
			      const std::string& newhash);
  //!@}

  /** @name Authenticaton attempt throttling
   */
  //!@{
  /*! \brief Sets the number of consecutive authentication failures.
   *
   * This sets the number of consecutive authentication failures since the
   * last valid login.
   *
   * This is used by the throttling logic to determine how much time a
   * user needs to wait before he can do a new login attempt.
   */
  virtual void setFailedLoginAttempts(const User& user, int count);

  /*! \brief Returns the number of consecutive authentication failures.
   *
   * \a setFailedLoginAttempts()
   */
  virtual int failedLoginAttempts(const User& user) const;

  /*! \brief Sets the time of the last login attempt.
   *
   * This sets the time at which the user attempted to login.
   */
  virtual void setLastLoginAttempt(const User& user, const WDateTime& t);

  /*! \brief Returns the time of the last login.
   *
   * \sa setLastLoginAttempt()
   */
  virtual WDateTime lastLoginAttempt(const User& user) const;
  //!@}

  /** @name Identity provider support
   */
  //!@{
  /*! \brief Returns the value of a claim for a user.
   *
   * Should return a null Json value when the claim is unavailable.
   */
  virtual Json::Value idpJsonClaim(const User& user, const std::string& claim) const;

  /*! \brief Adds a new Wt::Auth::IssuedToken to the database and returns it.
   S*/
  virtual Wt::Auth::IssuedToken idpTokenAdd(const std::string& value,
                                            const WDateTime& expirationTime,
                                            const std::string& purpose,
                                            const std::string& scope,
                                            const std::string& redirectUri,
                                            const User& user,
                                            const OAuthClient& authClient);

  /*! \brief Removes an issued token from the database.
   */
  virtual void idpTokenRemove(const IssuedToken& token);

  /*! \brief Finds a token in the database with a given value.
   */
  virtual IssuedToken idpTokenFindWithValue(const std::string& purpose, const std::string& value) const;

  /*! \brief Gets the expiration time for a token.
   */
  virtual WDateTime idpTokenExpirationTime(const IssuedToken& token) const;

  /*! \brief Gets the value for a token.
   */
  virtual std::string idpTokenValue(const IssuedToken& token) const;

  /*! \brief Gets the token purpose (authorization_code, access_token, id_token, refresh_token).
   */
  virtual std::string idpTokenPurpose(const IssuedToken& token) const;

  /*! \brief Gets the scope associated with the token.
   */
  virtual std::string idpTokenScope(const IssuedToken& token) const;

  /*! \brief Returns the redirect URI that was used with the token request.
   */
  virtual std::string idpTokenRedirectUri(const IssuedToken& token) const;

  /*! \brief Returns the user associated with the token.
   */
  virtual User idpTokenUser(const IssuedToken &token) const;

  /*! \brief Returns the authorization client (relying party) that is associated with the token.
   */
  virtual OAuthClient idpTokenOAuthClient(const IssuedToken &token) const;

  /*! \brief Finds the authorization client (relying party) with this identifier.
   */
  virtual OAuthClient idpClientFindWithId(const std::string& clientId) const;

  /*! \brief Returns the secret for this client.
   */
  virtual std::string idpClientSecret(const OAuthClient& client) const;

  /*! \brief Returns true if the given secret is correct for the given client.
   */
  virtual bool idpVerifySecret(const OAuthClient& client, const std::string& secret) const;

  /*! \brief Returns the redirect URI for this client.
   */
  virtual std::set<std::string> idpClientRedirectUris(const OAuthClient& client) const;

  /*! \brief Returns the identifier for this client.
   */
  virtual std::string idpClientId(const OAuthClient& client) const;

  /*! \brief Returns whether the client is confidential or public.
   */
  virtual bool idpClientConfidential(const OAuthClient& client) const;

  /*! \brief Returns the client authentication method (see OIDC Core chapter 9)
   */
  virtual ClientSecretMethod idpClientAuthMethod(const OAuthClient& client) const;

  /*! \brief Add a new client to the database and returns it.
   */
  virtual Wt::Auth::OAuthClient idpClientAdd(const std::string &clientId,
                                            bool confidential,
                                            const std::set<std::string> &redirectUris,
                                            ClientSecretMethod authMethod,
                                            const std::string &secret);
  //!@}
protected:
  AbstractUserDatabase();

private:
  AbstractUserDatabase(const AbstractUserDatabase&);
};

  }
}

#endif // WT_AUTH_ABSTRACT_USER_DATABASE
