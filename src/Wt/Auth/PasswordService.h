// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_PASSWORD_AUTH_H_
#define WT_AUTH_PASSWORD_AUTH_H_

#include <Wt/WValidator.h>
#include <Wt/Auth/AbstractPasswordService.h>

namespace Wt {
  namespace Auth {

/*! \class PasswordService Wt/Auth/PasswordService.h Wt/Auth/PasswordService.h
 *  \brief Password authentication service
 *
 * This class implements password authentication.
 *
 * Like all <b>service classes</b>, this class holds only
 * configuration state. Thus, once configured, it can be safely shared
 * between multiple sessions since its state (the configuration) is
 * read-only. 
 * \if cpp 
 * A "const PasswordService" object is thus thread-safe.
 * \endif
 *
 * Passwords are (usually) saved in the database using salted hash
 * functions. The process of computing new hashes, and verifying them
 * is delegated to an AbstractVerifier.
 *
 * The authentication class may be configured to enable password
 * attempt throttling. This provides protection against brute force
 * guessing of passwords. When throttling is enabled, new password
 * attempts are refused until the throttling period is finished.
 *
 * Password strength validation of a new user-chosen password may be
 * implemented by setting an AbstractStrengthValidator.
 *
 * \ingroup auth
 */
class WT_API PasswordService : public AbstractPasswordService
{
public:
  /*! \class AbstractVerifier
   *  \brief Abstract password hash computation and verification class
   *
   * This class defines the interface for verifying a passwords against
   * password hashes, or computing a new password hash for a password.
   *
   * \sa setVerifier()
   */
  class AbstractVerifier
  {
  public:
    /*! \brief Destructor.
     */
    virtual ~AbstractVerifier();

    /*! \brief Returns whether a password hash needs to be updated (recomputed).
     *
     * A \p hash may need to be updated if it has been computed with a
     * cryptographic method that is being disfavoured.
     */
    virtual bool needsUpdate(const PasswordHash& hash) const = 0;

    /*! \brief Computes the password hash for a clear text password.
     *
     * This must return a hash that can later be used to verify the
     * user's password, but which avoids compromising the user's password
     * in case of loss.
     */
    virtual PasswordHash hashPassword(const WString& password) const = 0;

    /*! \brief Verifies a password against a hash.
     *
     * This returns whether the given password matches with the user's
     * credentials stored in the hash.
     */
    virtual bool verify(const WString& password, const PasswordHash& hash) const
      = 0;
  };

  /*! \brief Constructor.
   *
   * Creates a new password authentication service, which depends on the
   * passed basic authentication service.
   */
  PasswordService(const AuthService& baseAuth);

  /*! \brief Destructor.
   */
  virtual ~PasswordService();

  /*! \brief Returns the basic authentication service.
   */
  virtual const AuthService& baseAuth() const override { return baseAuth_; }

  /*! \brief Sets a password verifier which computes authorization checks.
   *
   * The password verifier has as task to verify an entered password
   * against a password hash stored in the database, and also to create
   * or update a user's password hash.
   *
   * The default password verifier is \c 0.
   *
   * \sa verifyPassword(), updatePassword()
   */
  void setVerifier(std::unique_ptr<AbstractVerifier> verifier);

  /*! \brief Returns the password verifier.
   *
   * \sa setVerifier()
   */
  AbstractVerifier *verifier() const { return verifier_.get(); }

  /*! \brief Sets a validator which computes password strength.
   *
   * The default password strength validator is \c 0.
   */
  void setStrengthValidator
    (std::unique_ptr<AbstractStrengthValidator> validator);

  /*! \brief Returns the password strength validator.
   *
   * \sa setStrengthValidator()
   */
  virtual AbstractStrengthValidator *strengthValidator() const override
  { return validator_.get(); }

  /*! \brief Configures password attempt throttling.
   *
   * When password throttling is enabled, new password verification
   * attempts will be refused when the user has had too many
   * unsuccessful authentication attempts in a row.
   *
   * The exact back-off schema can be customized by specializing
   * getPasswordThrottle().
   */
  void setAttemptThrottlingEnabled(bool enabled);

  /*! \brief Returns whether password attempt throttling is enabled.
   *
   * \sa setAttemptThrottlingEnabled()
   */
  virtual bool attemptThrottlingEnabled() const override { return attemptThrottling_; }

  /*! \brief Returns the delay for this user for a next authentication
   *         attempt.
   *
   * \copydetails AbstractPasswordService::delayForNextAttempt()
   *
   * \sa setAttemptThrottlingEnabled(), getPasswordThrottle()
   */
  virtual int delayForNextAttempt(const User& user) const override;

  /*! \brief Verifies a password for a given user.
   *
   * \copydetails AbstractPasswordService::verifyPassword()
   *
   * \sa setVerifier(), setAttemptThrottlingEnabled()
   */
  virtual PasswordResult verifyPassword(const User& user,
					const WT_USTRING& password) const override;

  /*! \brief Sets a new password for the given user.
   *
   * This stores a new password for the user in the database.
   */
  virtual void updatePassword(const User& user, const WT_USTRING& password)
    const override;

protected:
  /*! \brief Returns how much throttle should be given considering a number of
   *         failed authentication attempts.
   *
   * The returned value is in seconds.
   *
   * The default implementation returns the following:
   * - failedAttempts == 0: 0
   * - failedAttempts == 1: 1
   * - failedAttempts == 2: 5
   * - failedAttempts == 3: 10
   * - failedAttempts > 3: 25
   */
  virtual int getPasswordThrottle(int failedAttempts) const;

private:
  PasswordService(const PasswordService&);

  const AuthService& baseAuth_;
  std::unique_ptr<AbstractVerifier> verifier_;
  std::unique_ptr<AbstractStrengthValidator> validator_;
  bool attemptThrottling_;
};

  }
}

#endif // WT_AUTH_PASSWORD_AUTH
