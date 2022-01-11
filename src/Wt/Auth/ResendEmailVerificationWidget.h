#ifndef RESENDEMAILVERIFICATIONDIALOG_H_
#define RESENDEMAILVERIFICATIONDIALOG_H_

#include "Wt/Auth/User.h"

#include "Wt/WTemplate.h"

namespace Wt {
  namespace Auth {

class AuthService;

/*! \class ResendEmailVerificationWidget Auth/ResendEmailVerificationWidget
 *  \brief A widget to resend the email verification email.
 *
 * The widget renders the <tt>"Wt.Auth.template.resend-email-verification"</tt>
 * template. It prompts for the email address and if it matches the
 * unverified email invokes AuthService::verifyEmailAddress().
 *
 * \sa AuthWidget::createResendEmailVerificationView()
 *
 * \ingroup auth
 */
class WT_API ResendEmailVerificationWidget : public WTemplate {
public:
  /*! \brief Constructor
   */
  ResendEmailVerificationWidget(const User& user, const AuthService& auth);

protected:
  /*! \brief Resend the email verification email.
   *
   * If the prompted email matches the unverified email address, the verification
   * email is sent.
   */
  void send();

  /*! \brief Removes this widget from the parent.
   */
  void cancel();

private:
  User user_;
  const AuthService& baseAuth_;
};

  }
}

#endif
