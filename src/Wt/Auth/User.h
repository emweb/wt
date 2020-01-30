// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_USER_H_
#define WT_AUTH_USER_H_

#include <Wt/WDateTime.h>
#include <Wt/WString.h>
#include <Wt/Auth/Token.h>
#include <Wt/Auth/PasswordHash.h>

namespace Wt {
  namespace Auth {

/*! \brief Enumeration for a user's account status.
 *
 * \sa status()
 */
enum class AccountStatus {
  Disabled, //!< Successfully identified but not allowed to log in.
  Normal    //!< Normal status
};

/*! \brief Enumeration for an email token stored for the user.
   */
enum class EmailTokenRole {
  VerifyEmail, //!< Token is used to verify his email address
  LostPassword //!< Token is used to allow the user to enter a new password
};

class AbstractUserDatabase;

/*! \class User Wt/Auth/User.h
 *  \brief A user
 *
 * This class represents a user. It is a value class that stores only
 * the user id and a reference to an AbstractUserDatabase to access
 * its properties.
 *
 * An object can point to a valid user, or be invalid. Invalid users
 * are typically used as return value for database queries which did
 * not match with an existing user.
 *
 * Not all methods are valid or applicable to your authentication
 * system. See AbstractUserDatabase for a discussion.
 *
 * \sa AbstractUserDatabase
 *
 * \ingroup auth
 */
class WT_API User
{
public:
  /*! \brief Typedef for enum Wt::Auth::AccountStatus */
  typedef AccountStatus Status;

  /*! \brief Default constructor.
   *
   * Creates an invalid user.
   *
   * \sa isValid()
   */
  User();

  /*! \brief Constructor.
   *
   * Creates a user with id \p id, and whose information is stored in
   * the \p database.
   */
  User(const std::string& id, const AbstractUserDatabase& database);

  /*! \brief Returns the user database.
   *
   * This returns the user database passed in the constructor, or 0 if the
   * user is invalid, and was constructed using the default constructor.
   */
  AbstractUserDatabase *database() const { return db_; }

  /*! \brief Comparison operator.
   *
   * Two users are equal if they have the same identity and the same database.
   */
  bool operator==(const User& other) const;

  /*! \brief Comparison operator.
   *
   * \sa operator==
   */
  bool operator!=(const User& other) const;

  /*! \brief Returns whether the user is valid.
   *
   * A invalid user is a sentinel value returned by methods that query
   * the database but could not identify a matching user.
   */
  bool isValid() const { return db_ != nullptr; }

  /*! \brief Returns the user id.
   *
   * This returns the id that uniquely identifies the user, and acts
   * as a "primary key" to obtain other information for the user in
   * the database.
   *
   * \sa AbstractUserDatabase
   */
  const std::string& id() const { return id_; }

  /*! \brief Returns an identity.
   */
  WT_USTRING identity(const std::string& provider) const;

  /*! \brief Adds an identity.
   *
   * Depending on whether the database supports multiple identities
   * per provider, this may change (like setIdentity()), or add
   * another identity to the user. For some identity providers (e.g. a
   * 3rd party identity provider), it may be sensible to have more
   * than one identity of the same provider for a single user
   * (e.g. multiple email accounts managed by the same provider, that
   * in fact identify the same user).
   */
  void addIdentity(const std::string& provider, const WT_USTRING& identity);

  /*! \brief Sets an identity.
   *
   * Unlike addIdentity() this overrides any other identity of the
   * given provider, in case the underlying database supports multiple
   * identities per user.
   */
  void setIdentity(const std::string& provider, const WT_USTRING& identity);

  /*! \brief Removes an identity.
   *
   * \sa addIdentity()
   */
  void removeIdentity(const std::string& provider);

  /*! \brief Sets a password.
   *
   * This also clears the email token.
   *
   * \sa AbstractUserDatabase::setPassword()
   * \sa clearEmailToken()
   */
  void setPassword(const PasswordHash& password) const;

  /*! \brief Returns the password.
   *
   * \sa AbstractUserDatabase::password()
   */
  PasswordHash password() const;

  /*! \brief Sets the email address.
   *
   * \sa AbstractUserDatabase::setEmail()
   */
  void setEmail(const std::string& address) const;

  /*! \brief Returns the email address.
   *
   * \sa AbstractUserDatabase::email()
   */
  std::string email() const;

  /*! \brief Sets the unverified email address.
   *
   * \sa AbstractUserDatabase::setUnverifiedEmail()
   */
  void setUnverifiedEmail(const std::string& address) const;

  /*! \brief Returns the unverified email address.
   *
   * \sa AbstractUserDatabase::unverifiedEmail()
   */
  std::string unverifiedEmail() const;

  /*! \brief Returns the account status.
   *
   * \sa AbstractUserDatabase::status()
   */
  AccountStatus status() const;

  /*! \brief Sets the account status.
   *
   * \sa AbstractUserDatabase::setStatus()
   */
  void setStatus(AccountStatus status);

  /*! \brief Returns the email token.
   *
   * \sa AbstractUserDatabase::emailToken()
   */
  Token emailToken() const;

  /*! \brief Returns the email token role.
   *
   * \sa AbstractUserDatabase::emailTokenRole()
   */
  EmailTokenRole emailTokenRole() const;

  /*! \brief Sets an email token.
   *
   * \sa AbstractUserDatabase::setEmailToken()
   */
  void setEmailToken(const Token& token, EmailTokenRole role) const;

  /*! \brief Clears the email token.
   *
   * \sa setEmailToken()
   */
  void clearEmailToken() const;

  /*! \brief Adds an authentication token.
   *
   * \sa AbstractUserDatabase::addAuthToken()
   */
  void addAuthToken(const Token& token) const;

  /*! \brief Removes an authentication token.
   *
   * \sa AbstractUserDatabase::removeAuthToken()
   */
  void removeAuthToken(const std::string& hash) const;

  /*! \brief Updates an authentication token.
   *
   * \sa AbstractUserDatabase::updateAuthToken()
   */
  int updateAuthToken(const std::string& hash, const std::string& newHash)
    const;

  /*! \brief Logs the result of an authentication attempt.
   *
   * This changes the number of failed login attempts, and stores the
   * current date as the last login attempt time.
   *
   * \sa failedLoginAttempts(), lastLoginAttempt()
   */
  void setAuthenticated(bool success) const;

  /*! \brief Returns the number of consecutive unsuccessful login attempts.
   *
   * \sa setAuthenticated()
   */
  int failedLoginAttempts() const;

  /*! \brief Returns the last login attempt.
   *
   * \sa setAuthenticated()
   */
  WDateTime lastLoginAttempt() const;

private:
  std::string id_;
  AbstractUserDatabase *db_;

  void checkValid() const;
};

  }
}

#endif // WT_AUTH_USER
