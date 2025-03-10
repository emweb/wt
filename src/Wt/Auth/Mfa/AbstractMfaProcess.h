// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_MFA_ABSTRACTMFAPROCESS_H_
#define WT_AUTH_MFA_ABSTRACTMFAPROCESS_H_

#include "Wt/Auth/AuthThrottle.h"

#include "Wt/WObject.h"
#include "Wt/WString.h"

namespace Wt {
  namespace Auth {
    namespace Mfa {

/*! \brief Enumeration that holds the result of the authentication event.
 *
 * The authentication event as a second step in the authentication flow,
 * will have a certain result after the user submits their input.
 */
enum class AuthenticationStatus
{
  //! Indicates a successful authentication event.
  Success,
  //! Indicates a failure to authenticate.
  Failure,
  //! An error was encountered when trying to authenticate.
  Error
};

/*! \brief A class that holds an authentication result.
 *
 * This class in essence is a record of an authentication attempt. The
 * AuthenticationStatus will indicate whether the event was successful or
 * not, and the optional string provides a way to customize a message.
 * This can be used to display more detailed information to the user, or
 * allow the developer to log some information.
 *
 * By default this signal is used in TotpProcess::processEnvironment().
 * There it is fired upon successfully matching the TOTP code and
 * finding a matching environment token (Http::Cookie) against the
 * database respectively.
 */
class WT_API AuthenticationResult
{
public:
  /*! \brief Default constructor.
   *
   * Creates an invalid result.
   */
  AuthenticationResult();

  /*! \brief Constructor.
   *
   * Creates a result with given \p status and \p message.
   */
  AuthenticationResult(AuthenticationStatus state, const WString& message);

  /*! \brief Constructor.
   *
   * Creates a result with given \p status and an empty message.
   */
  explicit AuthenticationResult(AuthenticationStatus status);

  //! Returns the authentication status.
  AuthenticationStatus status() const { return status_; }

  //! Returns the authentication message.
  const WString& message() const { return message_; }

private:
  AuthenticationStatus status_;
  WString message_;
};

/*! \class AbstractMfaProcess Wt/Auth/Mfa/AbstractMfaProcess.h
 * \brief The interface for a second factor authentication process.
 *
 * This class defines the interface to be used when implementing a
 * second factor in the authentication flow. Currently, it is strongly
 * advised (by among others [OWASP](https://cheatsheetseries.owasp.org/cheatsheets/Multifactor_Authentication_Cheat_Sheet.html#quick-recommendations))
 * that a second factor is added to any authentication module.
 *
 * When a developer wishes to enable MFA they will need to set the
 * AuthService::setMfaProvider(), and optionally
 * AuthService::setMfaRequired(). This will enable %Wt to display the
 * MFA step to users logging into the system. This will be shown after
 * a user:
 * - logs in (with username/password)
 * - is authenticated with a login cookie
 *
 * The MFA step can be either the setup view, which holds the initial
 * state for the MFA method. For %Wt's default, this means a QR code with
 * the TOTP secret encoded. This then adds an Identity to the user, by
 * which they can be authenticated against a certain provider. The secret
 * key is also displayed, so that it can be copied into a password manager
 * or authenticator app.
 *
 * For this to work, a developer needs to implement createSetupView().
 * The method creates the widget that displays the view. This view tells
 * the user how to set up the MFA step. The default TotpProcess will
 * bind the content to a template. But developers are free to use their
 * own way (like showing a pop-up) This will require the developer to
 * override the AuthWidget::createMfaView().
 *
 * After this setup, the process can be repeated but a simpler view is
 * often desired here, one that no longer hold any configuration state,
 * but simply asks for a token of authentication. This is where
 * createInputView() comes into play. This functions the exact same way
 * as the above setup view counterpart, but shows less information.
 * Again, for %Wt's default implementation, using TOTP, this
 * means a 6 (depending on configuration, see:
 * AuthService::setMfaCodeLength()) digit code, that is generated from
 * their secret key will be asked of the user. The initial (QR) code
 * that serves as the way to generate the TOTP keys, will no longer be
 * displayed.
 *
 * A successful match of the second factor will then result in an actual
 * login (Login::login()) (see the note).
 *
 * To use your own widget in the normal authentication flow, one also
 * needs to override AuthWidget::createMfaProcess(), so that it will create
 * the correct widget. By default this will create the TotpProcess.
 *
 * \note The Login::changed() signal will be fired both when the user logs
 * in with username/password, and when the MFA step is completed
 * successfully. If your application listens to this signal to determine
 * some state or logic, you should check whether the login has taken place
 * fully (based on the LoginState (Login::state())). For convenience it's
 * a good idea to make your custom widget fire a signal when it tries to
 * authenticate \sa AuthenticationResult.
 */
class WT_API AbstractMfaProcess : public WObject
{
public:
  //! Constructor
  AbstractMfaProcess(const AuthService& authService, AbstractUserDatabase& users, Login& login);

  virtual ~AbstractMfaProcess();

  /*! \brief Returns the name of the provider for the process.
   *
   *\sa AuthService::setMfaProvider()
   */
  virtual const std::string& provider() const;

  /*! \brief Processes the (initial) environment.
   *
   * This can be called to tell the widget to look through the
   * environment for the relevant cookies. It will handle the
   * side-effect of finding such a cookie, and it still being valid. The
   * user will be logged in, in a weak state (LoginState::Weak), and the
   * authenticated() signal will be fired, with an
   * AuthenticationStatus::Success.
   */
  virtual void processEnvironment();

  /*! \brief Creates the view that displays the MFA configuration step.
   *
   * This is the view that is shown to a user if they do not have MFA
   * enabled yet. This will often show more information to them, telling
   * them how the feature is to be used and activated.
   *
   * The state of whether a user has MFA enabled or not can be decided
   * in two ways:
   * - the feature is enabled (AuthService::mfaProvider() isn't empty)
   *   and they have an identity for the provider.
   *   By default %Wt's TOTP implementation will take the
   *   Identity::MultiFactor name as the provider.
   * - the feature is enabled AND required (AuthService::mfaRequired()
   *   is set to \p true)
   */
  virtual std::unique_ptr<WWidget> createSetupView() = 0;

  /*! \brief Creates the view that displays the MFA input step.
   *
   * The user already has an identity attached to their record. This step
   * now needs valid input from them to continue.
   *
   * \sa createSetupView
   */
  virtual std::unique_ptr<WWidget> createInputView() = 0;

  /*! \brief Sets the instance that manages throttling.
   *
   * Throtteling is an additional safety measure. It ensures that the
   * MFA process cannot be brute-forced.
   *
   * Setting the throttler, will allow for it to be configured
   * (configureThrottling()), and updated (updateThrottling()) if
   * applicable.
   */
  void setMfaThrottle(std::unique_ptr<AuthThrottle> authThrottle);

protected:
  /*! \brief Retrieves the current User's identity for the provider.
   *
   * This is simply a method that retrieves the current User's identity,
   * given the provider the process specified (see provider()). This can
   * be accessed by calling User::identity() as well.
   *
   * The method will return the identity, if it exists, or an empty
   * string if it does not. It will also log (to "warn") in the latter
   * case.
   *
   * \sa createUserIdentity()
   */
  virtual Wt::WString userIdentity();

  /*! \brief Adds an Identity to the current User with the given value.
   *
   * The identity will be created with the specified provider() on the
   * process. And the actual identity will be \p identityValue.
   *
   * This is again a method that offers very basic functionality, calling:
   * User::identity() and User::addIdentity(), with some logging added to
   * it.
   *
   * \note This will store the value in plaintext in the database. Should
   * your chosen method of MFA want this to be stored in a more secure
   * manner, the developer will have to do this manually (or the database
   * itself should be encrypted).
   *
   * \sa userIdentity()
   */
  virtual bool createUserIdentity(const Wt::WString& identityValue);

  /*! \brief Processes an MFA authentication token
   *
   * If a token is present in the browser, going by the name found in
   * AuthService::mfaTokenCookieName(), and that is still valid
   * (see AuthService::mfaTokenValidity()), the User can be retrieved
   * from that token. This identifies the user uniquely, ensuring
   * their MFA verification step can be skipped for a certain period.
   */
  virtual User processMfaToken();

  /*! \brief Creates an MFA authentication token.
   *
   * A token (with the correct prefix for MFA) is created and persisted
   * in the database. A cookie is created in the \p user's browser.
   * This token can later be used by processMfaToken() to identify the
   * User, allowing them to skip the MFA step.
   */
  virtual void setRememberMeCookie(User user);

  /*! \brief Configures client-side throttling on the process.
   *
   * If attempt throttling is enabled, then this may also be
   * indicated client-side using JavaScript by disabling the login
   * button and showing a count-down indicator. This method
   * initializes this JavaScript utility function for a login button.
   *
   * If throttling is enabled, it may be necessary for a custom
   * implementation to manage this state itself. This is to allow
   * developers the freedom to define their own MFA processes.
   *
   * Look at TotpProcess::verifyCode() for an example.
   *
   * \sa updateThrottling()
   */
  virtual void configureThrottling(WInteractWidget* button);

  /*! \brief Updates client-side login throttling on the process.
   *
   * This should be called after a MFA authentication event takes place,
   * if you want to reflect throttling using a client-side count-down
   * indicator on the button.
   *
   * You need to call configureThrottling() before you can do this.
   *
   * If throttling is enabled, it may be necessary for a custom
   * implementation to manage this state itself. This is to allow
   * developers the freedom to define their own MFA processes.
   *
   * Look at TotpProcess::verifyCode() for an example.
   */
  virtual void updateThrottling(WInteractWidget* button);

  const AuthService& baseAuth() const { return baseAuth_; }
  AbstractUserDatabase& users() const { return users_; }
  Login& login() const { return login_; }

  int throttlingDelay_;
  WT_NODISCARD AuthThrottle* mfaThrottle() const { return mfaThrottle_.get(); }

private:
  const AuthService& baseAuth_;
  AbstractUserDatabase& users_;
  Login& login_;

  std::unique_ptr<AuthThrottle> mfaThrottle_;
};
    }
  }
}
#endif // WT_AUTH_MFA_ABSTRACTMFAPROCESS_H_
