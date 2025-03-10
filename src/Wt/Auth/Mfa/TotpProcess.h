// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_MFA_TOTPPROCESS_H_
#define WT_AUTH_MFA_TOTPPROCESS_H_

#include "Wt/Auth/Mfa/AbstractMfaProcess.h"

#include "Wt/WCheckBox.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WLineEdit.h"

namespace Wt {
  namespace Auth {
    class AbstractUserDatabase;
    class Login;

    namespace Mfa {
  /*! \class TotpProcess Wt/Auth/Mfa/TotpProcess
   *  \brief A process managing the TOTP setup and validation.
   *
   * This process allows the User that is trying to log in an extra step
   * to validate their identity. They will have to provide the TOTP code
   * as an extra step to validate their log-in additionally.
   *
   * This process' functionality is twofold. If a User does not have TOTP
   * set-up yet, they will be shown the QR code and the secret key. They
   * can then add this to an authenticator app or extension of their
   * choice (createSetupView()).
   *
   * If they have it enabled already, they will simply be asked to provide
   * a TOTP code to verify their identity as a second factor
   * (createInputView()).
   *
   * This process will also look in the environment for cookies, so that
   * the MFA step can also be remembered (processEnvironment()). If the
   * user logs in using a device they use often, they can opt to remember
   * the login, which creates a cookie
   * (see AuthService::setMfaTokenCookieName(),
   * and AuthService::setMfaTokenValidity()). If this cookie remains in
   * the user's browser, and in the local database (in the "auth_token"
   * table, by default), the user's TOTP step can be skipped for a
   * certain time (see AbstractMfaProcess::processMfaToken()).
   *
   * If a developer wants to force all users to use this functionality,
   * they can do so by enabling AuthService::setMfaRequired().
   *
   * Whether or not this process is executed when logging in, is managed
   * by AuthModel::hasMfaStep().
   *
   * \sa AuthService::setMfaRequired()
   * \sa validateCode()
   */
  class WT_API TotpProcess : public AbstractMfaProcess
  {
  public:
    /*! \brief Constructs the TotpProcess holding the TOTP "login".
     *
     * For the provided authentication service \p authService this will
     * either request a TOTP code from the user as a second factor, or
     * initiate the process to add the TOTP secret to their record,
     * allowing for future TOTP code requests.
     *
     * Optionally, if authentication tokens are enabled
     * (see AuthService::setAuthTokensEnabled()), this step can be
     * temporarily bypassed, for as long as the token is valid
     * (see AuthService::mfaTokenValidity()).
     */
    TotpProcess(const AuthService& authService, AbstractUserDatabase& users, Login& login);

    //! Destructor
    virtual ~TotpProcess();

    void processEnvironment() override;

    /*! \brief Creates the view to manage the TOTP code.
     *
     * This either adds a new code to the user, or expects a code to be
     * entered based on their existing TOTP secret key.
     */
    std::unique_ptr<WWidget> createSetupView() override;
    //! Creates the view to input the TOTP code.
    std::unique_ptr<WWidget> createInputView() override;

    /*! \brief Signal emitted upon an authentication event
     *
     * This event can be a success, failure, or error.
     *
     * The additional string can provide more information on the attempt,
     * indicating the type of error, or the reason for the failure.
     * The status and message are both stored in an instance of the
     * AuthenticationResult.
     *
     * This can be used to reliably check whether the user has logged in
     * with MFA. Previously the Login::changed() signal had been fired,
     * when the user logged in, but it could still be that the state was
     * not LoginState::Weak or LoginState::Strong, but
     * LoginState::RequiresMfa. This signal can be listened to, to ensure
     * that, on success, the user will actually be logged in.
     *
     * Side-effects to the login can then be attached to this signal.
     */
    Signal<AuthenticationResult>& authenticated() { return authenticated_; }

  protected:
    /*! \brief Creates the base of the UI.
     *
     * This retrieves the message <code>Wt.Auth.template.totp</code>
     * from Wt's resource bundle. This is used as the main template
     * to build the UI from, and bind all elements to it.
     */
    virtual std::unique_ptr<WTemplate> createBaseView();

    /*! \brief Binds the QR Code to the UI.
     *
     * This binds both the rendered QR code, as well as the
     * secret itself (to be copied over to a authenticator app).
     *
     * \sa createBaseView()
     */
    virtual void bindQRCode(WTemplate* view);

    /*! \brief Binds the code input field to the UI.
     *
     * This creates and binds the field where the generated TOTP code
     * will be entered.
     *
     * The \p throttle parameter manages whether or not pressing "enter"
     * on this field will result in the verification of the code being
     * subject to throttling (see AuthThrottle).
     *
     * \sa createBaseView()
     */
    virtual void bindCodeInput(WTemplate* view, bool throttle);

    /*! \brief Binds "remember-me" checkbox to the UI.
     *
     * This creates and binds the checkbox which enables "remember-me"
     * functionality, if this is enabled (see
     * AuthService::setAuthTokensEnabled()).
     *
     * \sa createBaseView()
     */
    virtual void bindRememberMe(WTemplate* view);

    /*! \brief Binds the "Login" button to the UI.
     *
     * This button will run the verification process on the entered TOTP
     * code.
     *
     * The \p throttle parameter manages whether or not pressing the
     * button will result in the verification of the code being subject
     * to throttling (see AuthThrottle).
     *
     * \sa createBaseView()
     */
    virtual void bindLoginButton(WTemplate* view, bool throttle);

    /*! \brief Binds the "Back" button to the UI.
     *
     * This button can be pressed to return to the original login page.
     * This may be handy in case the user isn't able to provide the TOTP
     * code.
     *
     * \sa createBaseView()
     */
    virtual void bindLogoutButton(WTemplate* view);

    /*! \brief Updates the UI after a verification attempt.
     *
     * This updates the \p view with the effects of the verification
     * attempt. Displaying a message in case it did not succeed, and
     * updating the styling of the input field.
     *
     * The \p throttle parameter manages whether or not the attempt will
     * result in the verification of the code being subject to
     * throttling (see AuthThrottle).
     *
     * \sa createBaseView()
     */
    virtual void update(WTemplate* view, bool throttle);

    /*! \brief Verifies the entered code.
     *
     * The code the user provided is verified, and the UI is updated
     * accordingly (using update()).
     *
     * This also handles the actual authentication, in case the
     * verification is successful. This logs in the user (see
     * Login::login()), and fires the authenticated() signal.
     *
     * In case of the "remember-me" functionality being enabled, it will
     * create the cookie.
     */
    virtual void verifyCode(WTemplate* view, bool throttle);

    /*! \brief The secret key of the user.
     *
     * Either this is a newly generated key, in case the user is setting
     * up their TOTP access. Or this is the one stored in the database.
     * In case of the latter being true, be careful not to display this
     * in the browser.
     */
    const std::string& currentSecretKey() const { return currentSecretKey_; }

  private:
    WLineEdit* codeEdit_ = nullptr;
    WCheckBox* rememberMeField_ = nullptr;

    std::string currentSecretKey_;

    Signal<AuthenticationResult> authenticated_;
  };
    }
  }
}
#endif // WT_AUTH_MFA_TOTPPROCESS_H_
