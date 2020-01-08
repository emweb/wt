// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_AUTH_MODEL_H_
#define WT_AUTH_AUTH_MODEL_H_

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/FormBaseModel.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/User.h>

#include <memory>

namespace Wt {
  namespace Auth {

class AbstractPasswordService;
class AbstractUserDatabase;
class Login;
class OAuthService;

/*! \class AuthModel Wt/Auth/AuthModel.h
 *  \brief Model for implementing an authentication view.
 *
 * This model implements the logic for authenticating a user (the
 * "login" interface). It implements traditional username/password
 * registration, and third party identification methods (although for
 * the latter, it doesn't really do anything).
 *
 * The model exposes three fields:
 * - LoginNameField: the login name (used as an identity for the
 *   Identity::LoginName provider)
 * - PasswordField: the password
 * - RememberMeField: whether the login should be remembered with an
 *   authentication cookie (if that is configured in the AuthService).
 *
 * When the model validates correctly (validate() returns \c true),
 * the entered credentials are correct. At that point you can use the
 * login() utility function to login the identified user.
 *
 * The model can also be used when the user is already known (e.g. to
 * implement password confirmation before a critical operation). In that
 * case you can set a value for the LoginNameField and make this field
 * invisible or read-only.
 *
 * The model also provides the client-side JavaScript logic to
 * indicate password attempt throttling (configureThrottling() and
 * updateThrottling()).
 *
 * \sa AuthWidget
 *
 * \ingroup auth
 */
class WT_API AuthModel : public FormBaseModel
#ifndef WT_TARGET_JAVA
                         , public std::enable_shared_from_this<AuthModel>
#endif // WT_TARGET_JAVA
{
public:
  //! \brief Password field
  static const Field PasswordField;

  //! \brief Remember-me field
  static const Field RememberMeField;

  /*! \brief Constructor.
   *
   * Creates a new authentication model, using a basic authentication
   * service and user database.
   */
  AuthModel(const AuthService& baseAuth, AbstractUserDatabase& users);

  virtual void reset() override;
  virtual bool isVisible(Field field) const override;
  virtual bool validateField(Field field) override;
  virtual bool validate() override;

  /*! \brief Initializes client-side login throttling.
   *
   * If login attempt throttling is enabled, then this may also be
   * indicated client-side using JavaScript by disabling the login
   * button and showing a count-down indicator. This method
   * initializes this JavaScript utlity function for a login button.
   *
   * \sa updateThrottling()
   */
  virtual void configureThrottling(WInteractWidget *button);

  /*! \brief Updates client-side login throttling.
   *
   * This should be called after a call to attemptPasswordLogin(), if
   * you want to reflect throttling using a client-side count-down
   * indicator in the button.
   *
   * You need to call configureThrottling() before you can do this.
   */
  virtual void updateThrottling(WInteractWidget *button);

  /*! \brief Logs the user in.
   *
   * Logs in the user after a successful call to validate(). To avoid
   * mishaps, you should call this method immediately after a call to
   * validate().
   *
   * Returns whether the user could be logged in.
   */
  virtual bool login(Login& login);

  /*! \brief Logs the user out.
   *
   * This also removes the remember-me cookie for the user.
   */
  virtual void logout(Login& login);

  /*! \brief Processes an email token.
   *
   * This simply calls AuthService::processEmailToken().
   */
  virtual EmailTokenResult processEmailToken(const std::string& token);

  /*! \brief Creates a token and stores it in a cookie.
   *
   * This enables automatic authentication in a next session.
   */
  virtual void setRememberMeCookie(const User& user);

  /*! \brief Detects and processes an authentication token.
   *
   * This returns a user that was identified with an authentication token
   * found in the application environment, or an invalid User
   * object if this feature is not configured, or no valid cookie was found.
   *
   * \sa AuthService::processAuthToken()
   */
  virtual User processAuthToken();

private:
  int throttlingDelay_;
};

  }
}

#endif // WT_AUTH_AUTH_MODEL_H_
