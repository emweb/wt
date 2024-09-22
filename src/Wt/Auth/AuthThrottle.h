#ifndef WT_AUTH_AUTH_THROTTLE_H
#define WT_AUTH_AUTH_THROTTLE_H

#include "Wt/Auth/User.h"

#include "Wt/WInteractWidget.h"

namespace Wt {
  namespace Auth {
/*! \class AuthThrottle Wt/Auth/AuthThrottle.h
 *  \brief Class with helper functions to define throttling behaviour
 *
 * During authentication, it may be desirable to limit the number of
 * submissions a client can perform in order to prevent brute force
 * attacks. This class allows developers to manage the throttling
 * behaviour. The main functions offered by this class are:
 * 1. compute how long a user has to wait before the next authentication
 *    attempt
 * 2. offer methods to show interactive feedback (count-down timer)
 *    on the waiting time to the user
 *
 * This class is used in PasswordService and in AbstractMfaProcess. By
 * specializing this class, you can change Wt's behaviour related to
 * authentication throttling.
 *
 * Throttling has to be enabled; it is enabled by calling
 * PasswordService::setAttemptThrottlingEnabled() for password
 * authentication, or AuthService::setMfaThrottleEnabled() for MFA
 * tokens.
 *
 * By default this is disabled in both cases.
 *
 * \note for MFA one can also call
 * Mfa::AbstractMfaProcess::setMfaThrottle() directly. The AuthService
 * state can be used for the default TotpProcess implementation, or to
 * manage your own state in case of a custom MFA approach.
 */
class WT_API AuthThrottle {
public:
  //! Default constructor
  AuthThrottle() = default;
  //! Default destructor
  virtual ~AuthThrottle() = default;

  /*! \brief Returns the amount of seconds the given User has to wait
   *         for the next authentication attempt.
   *
   * The default implementation queries the given user object for the
   * last login attempt (User::lastLoginAttempt()) and the amount of
   * consecutive failed logins (User::failedLoginAttempts()),
   * to compute the remaining wait time for this user. This is
   * calculated using getAuthenticationThrottle() and the current server
   * time.
   *
   * Since the User object normally fetches this data from a database,
   * throttling works across different sessions, i.e. opening a new tab
   * to get around throttling will not work.
   *
   * This function returns 0 if a new attempt is permitted, or the
   * amount of seconds the user has to wait for the next attempt.
   *
   * \sa AbstractPasswordService::attemptThrottlingEnabled(),
   * Mfa::AbstractMfaProcess::setMfaThrottle()
   */
  virtual int delayForNextAttempt(const User& user) const;

  /*! \brief Returns the number of seconds a user needs to wait
   *         between two authentication attempts, given the
   *         amount of failed attempts since the last successful
   *         login.
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
  virtual int getAuthenticationThrottle(int failedAttempts) const;

  /*! \brief Prepare a widget for showing client-side updated
   *         throttling message
   *
   * This method loads the necessary JavaScript and initializes a
   * widget in preparation of calls to updateThrottlingMessage().
   *
   * The widget's inner HTML will be replaced by a count-down message,
   * defined in <tt>`Wt.Auth.throttle-retry`</tt>, for the duration of
   * the wait time. In addition, the widget is disabled, so that the
   * user cannot click it. When the count-down reaches 0, The original
   * content of the widget is restored and the widget is re-enabled.
   *
   * This will work with widgets such as buttons or anchors, which are
   * usually used to implement button-style functionality.
   *
   * \sa updateThrottlingMessage()
   */
  virtual void initializeThrottlingMessage(WInteractWidget* button);

  /*! \brief Show the count-down message and disable the widget for the
   *         given delay.
   *
   * The \p delay is specified in seconds.
   *
   * This method is called after an attempt to authenticate, to inform the
   * user with a client-side count-down indicator in the widget. This method
   * should disable the widget, and show a count-down text to the user.
   *
   * Each second the counter will be updated, and the number of seconds to
   * wait for will decrease by one, eventually enabling the widget again.
   *
   * You need to call configureThrottling() before you can do this. See
   * the documentation of configureThrottling() for more information on the
   * default behaviour of this method.
   *
   * \note Throttling delay must always be verified server-side
   *
   * \sa delayForNextAttempt()
   */
  virtual void updateThrottlingMessage(WInteractWidget* button, int delay);
};
  }
}

#endif // WT_AUTH_AUTH_THROTTLE_H
