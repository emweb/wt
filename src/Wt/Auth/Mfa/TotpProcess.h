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

class AbstractUserDatabase;
class Login;

namespace Wt {
  namespace Auth {
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
  class TotpProcess : public AbstractMfaProcess
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

  private:
    WLineEdit* codeEdit_ = nullptr;
    WCheckBox* rememberMeField_ = nullptr;

    std::string currentSecretKey_;

    Signal<AuthenticationResult> authenticated_;

    std::unique_ptr<WTemplate> createBaseView();

    void bindQRCode(WTemplate* view);
    void bindCodeInput(WTemplate* view, bool throttle);
    void bindRememberMe(WTemplate* view);
    void bindLoginButton(WTemplate* view, bool throttle);
    void bindLogoutButton(WTemplate* view);
    void verifyCode(WTemplate* view, bool throttle);
    void update(WTemplate* view, bool throttle);

    const std::string& currentSecretKey() const { return currentSecretKey_; }
  };
    }
  }
}
#endif // WT_AUTH_MFA_TOTPPROCESS_H_
