// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_ABSTRACT_PASSWORD_AUTH_H_
#define WT_AUTH_ABSTRACT_PASSWORD_AUTH_H_

#include <Wt/WValidator.h>
#include <Wt/WString.h>

#include <Wt/Auth/User.h>

namespace Wt {
  namespace Auth {

class AuthService;
class User;

/*! \brief Enumeration for a password verification result
 *
 * \sa AbstractPasswordService::verifyPassword()
 *
 * \ingroup auth
 */
enum class PasswordResult {
  PasswordInvalid, //!< The password is invalid
  LoginThrottling, //!< The attempt was not processed because of throttling
  PasswordValid    //!< The password is valid
};

/*! \class AbstractPasswordService Wt/Auth/AbstractPasswordService.h
 *  \brief Abstract password authentication service
 *
 * This abstract class defines the interface for password authentication.
 *
 * It provides methods to verify a password, to update a password, and to
 * throttle password verification attempts.
 *
 * \sa PasswordService a default implementation
 *
 * \ingroup auth
 */
class WT_API AbstractPasswordService
{
public:
  /*! \class StrengthValidatorResult
   *  \brief Result returned when validating password strength.
   *
   * This class contains information on the validity and the strength of 
   * the password together with possible messages.
   * When the password is considered not strong enough, a message should 
   * be provided which helps the user pick a stronger password.
   *
   * \sa AbstractStrengthValidator::evaluateStrength()
   */
  class StrengthValidatorResult {
  public:
    /*! \brief Constructor.
     */
    StrengthValidatorResult(bool valid, 
			    const WString &message,
			    int strength);
    
    /*! \brief Returns whether the password is considered strong enough.
     */
    bool isValid() { return valid_; }

    /*! \brief Returns a message describing the password strength.
     */
    WString message() { return message_; }

    /*! \brief Returns the password strength in a scale of 0 to 5.
     */
    int strength() { return strength_; }
    
  private:
    bool valid_;
    WString message_;
    int strength_;
  };

  /*! \class AbstractStrengthValidator
   *  \brief Validator for password strength.
   *
   * This class defines a specialized validator interface for evaluating
   * password strength. The implementation allows to evaluate strength 
   * in addition to the normal validator functionality of validating a 
   * password.
   *
   * The evaluateStrength() computes the strength and returns
   * an instance of StrenghtValidatorResult which contains information on the 
   * validity and the strength of the password together with possible messages.
   *
   * \sa strengthValidator()
   */
  class WT_API AbstractStrengthValidator : public Wt::WValidator
  {
  public:
    /*! \brief Constructor.
     */
    AbstractStrengthValidator();

    /*! \brief Evaluates the strength of a password.
     *
     * The result is an instance of StrengthValidatorResult which 
     * contains information on the validity and the strength of the password 
     * together with possible messages.
     *
     * The validator may take into account the user's login name and
     * email address, to exclude passwords that are too similar to
     * these.
     */
    virtual StrengthValidatorResult 
      evaluateStrength(const WT_USTRING& password,
		       const WT_USTRING& loginName,
		       const std::string& email) const = 0;

    /*! \brief Validates a password.
     *
     * This uses evaluateStrength(), isValid() and message() to return the
     * result of password validation.
     */
    virtual Result validate(const WT_USTRING& password,
			    const WT_USTRING& loginName,
			    const std::string& email) const;

    /*! \brief Validates a password.
     *
     * Calls validate(password, WString::Empty, "");
     */
    virtual Result validate(const WT_USTRING& password) const override;
  };

  /*! \brief Destructor.
   */
  virtual ~AbstractPasswordService();

  /*! \brief Returns the basic authentication service.
   */
  virtual const AuthService& baseAuth() const = 0;

  /*! \brief Returns whether password attempt throttling is enabled.
   */
  virtual bool attemptThrottlingEnabled() const = 0;

  /*! \brief Returns a validator which checks that a password is strong enough.
   */
  virtual AbstractStrengthValidator *strengthValidator() const = 0;

  /*! \brief Returns the delay for this user for a next authentication
   *         attempt.
   *
   * If password attempt throttling is enabled, then this returns the
   * number of seconds this user must wait for a new authentication
   * attempt, presumably because of a number of failed attempts.
   *
   * \sa attemptThrottlingEnabled()
   */
  virtual int delayForNextAttempt(const User& user) const = 0;

  /*! \brief Verifies a password for a given user.
   *
   * The supplied password is verified against the user's credentials
   * stored in the database. If password account throttling is
   * enabled, it may also refuse an authentication attempt.
   *
   * \sa setVerifier(), setAttemptThrottlingEnabled()
   */
  virtual PasswordResult verifyPassword(const User& user,
					const WT_USTRING& password) const = 0;

  /*! \brief Sets a new password for the given user.
   *
   * This stores a new password for the user in the database. 
   */
  virtual void updatePassword(const User& user, const WT_USTRING& password)
    const = 0;
};

  }
}

#endif // WT_AUTH_ABSTRACT_PASSWORD_AUTH
