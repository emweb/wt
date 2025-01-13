#include "TotpProcess.h"

#include "Totp.h"
#include "TotpQrCode.h"

#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Login.h"

#include "Wt/WApplication.h"
#include "Wt/WCheckBox.h"
#include "Wt/WDateTime.h"
#include "Wt/WInteractWidget.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"

#include "Configuration.h"

namespace Wt {
  LOGGER("Auth.Mfa.TotpProcess");

  namespace Auth {
    namespace Mfa {
  TotpProcess::TotpProcess(const AuthService& authService, AbstractUserDatabase& users, Login& login)
    : AbstractMfaProcess(authService, users, login)
  {
  }

  TotpProcess::~TotpProcess()
  {
  }

  void TotpProcess::processEnvironment()
  {
    User user = processMfaToken();
    if (user.isValid()) {
      login().login(user, LoginState::Weak);
      authenticated_.emit(AuthenticationResult(AuthenticationStatus::Success));
    }
  }

  std::unique_ptr<WTemplate> TotpProcess::createBaseView()
  {
    auto baseView = std::make_unique<WTemplate>(WTemplate::tr("Wt.Auth.template.totp"));
    baseView->addFunction("id", &WTemplate::Functions::id);
    baseView->addFunction("tr", &WTemplate::Functions::tr);
    baseView->addFunction("block", &WTemplate::Functions::block);
    return baseView;
  }

  std::unique_ptr<WWidget> TotpProcess::createSetupView()
  {
    auto setupView = createBaseView();
    bindQRCode(setupView.get());
    bindCodeInput(setupView.get(), false);
    bindRememberMe(setupView.get());
    bindLoginButton(setupView.get(), false);
    bindLogoutButton(setupView.get());
    return setupView;
  }

  std::unique_ptr<WWidget> TotpProcess::createInputView()
  {
    auto inputView = createBaseView();
    bindCodeInput(inputView.get(), true);
    bindRememberMe(inputView.get());
    bindLoginButton(inputView.get(), true);
    bindLogoutButton(inputView.get());
    return inputView;
  }

  void TotpProcess::bindQRCode(WTemplate* view)
  {
    auto totpSecretKey = userIdentity();
    if (totpSecretKey.empty()) {
#ifndef WT_TARGET_JAVA
      currentSecretKey_ = generateSecretKey();
#else
      currentSecretKey_ = Totp::generateSecretKey();
#endif // WT_TARGET_JAVA
      view->setCondition("if:no-secret-key", true);
    } else {
      currentSecretKey_ = totpSecretKey.toUTF8();
    }

    view->bindWidget("qr-code", std::make_unique<TotpQrCode>(currentSecretKey(), baseAuth().mfaProvider(), login().user().email(), baseAuth().mfaCodeLength()));
    view->bindString("secret-key", currentSecretKey());
  }

  void TotpProcess::bindCodeInput(WTemplate* view, bool throttle)
  {
    codeEdit_ = view->bindNew<WLineEdit>("totp-code");
    codeEdit_->setFocus(true);
    codeEdit_->enterPressed().connect(std::bind(&TotpProcess::verifyCode, this, view, throttle));
    view->bindString("totp-code-info", WString::tr("Wt.Auth.totp-code-info"));

    auto totpSecretKey = userIdentity();
    if (currentSecretKey_.empty() && !totpSecretKey.empty()) {
      currentSecretKey_ = totpSecretKey.toUTF8();
    } else if (currentSecretKey_.empty() && totpSecretKey.empty()) {
      LOG_ERROR("createCodeInput: No secret key was set, or could be retrieved from the database.");
    }
  }

  void TotpProcess::bindLoginButton(WTemplate* view, bool throttle)
  {
    auto login = view->bindNew<WPushButton>("login", WString::tr("Wt.Auth.login"));
    login->clicked().connect(std::bind(&TotpProcess::verifyCode, this, view, throttle));

    if (mfaThrottle() && throttle) {
      configureThrottling(login);
    }
  }

  void TotpProcess::bindLogoutButton(WTemplate* view)
  {
    auto logout = view->bindNew<WPushButton>("logout", WString::tr("Wt.Auth.totp-back"));
    logout->clicked().connect(std::bind(&Login::logout, &login()));
  }

  void TotpProcess::bindRememberMe(WTemplate* view)
  {
    view->setCondition("if:remember-me", true);
    rememberMeField_ = view->bindNew<WCheckBox>("remember-me");

    int days = baseAuth().mfaTokenValidity() / 24 / 60;

    WDateTime currentDateTime = WDateTime::currentDateTime();
    WDateTime expirationDateTime = currentDateTime.addDays(days);
    WString info = WString::tr("Wt.Auth.remember-me-info.dynamic").arg(currentDateTime.timeTo(expirationDateTime));

    view->bindString("remember-me-info", info);
  }

  void TotpProcess::verifyCode(WTemplate* view, bool throttle)
  {
    auto code = codeEdit_->text().toUTF8();
#ifndef WT_TARGET_JAVA
    auto validation = validateCode(currentSecretKey(), code, baseAuth().mfaCodeLength(), std::chrono::seconds(WDateTime::currentDateTime().toTime_t()));
#else
    auto validation = Totp::validateCode(currentSecretKey(), code, baseAuth().mfaCodeLength(), std::chrono::seconds(WDateTime::currentDateTime().toTime_t()));
#endif // WT_TARGET_JAVA

    LOG_INFO("verifyCode(): The validation resulted in " << (validation ? "success" : "failure") << " for user: " << login().user().id());

    if (mfaThrottle() && throttle) {
      throttlingDelay_ = mfaThrottle()->delayForNextAttempt(login().user());
      if (throttlingDelay_ > 0) {
        validation = false;
      }
    }

    std::unique_ptr<AbstractUserDatabase::Transaction> t(users().startTransaction());
    login().user().setAuthenticated(validation);
    t->commit();

    if (!validation) {
      if (throttlingDelay_ > 0) {
        update(view, throttle);
        authenticated_.emit(AuthenticationResult(AuthenticationStatus::Failure, WString::tr("Wt.Auth.totp-code-info-throttle")));
        return;
      }

      update(view, throttle);
      authenticated_.emit(AuthenticationResult(AuthenticationStatus::Failure, WString::tr("Wt.Auth.totp-code-info-invalid")));
    } else {
      createUserIdentity(currentSecretKey());

      if (rememberMeField_->isChecked()) {
        setRememberMeCookie(login().user());
      }

      login().login(login().user());
      authenticated_.emit(AuthenticationResult(AuthenticationStatus::Success));
    }
  }

  void TotpProcess::update(WTemplate* view, bool throttle)
  {
    codeEdit_->addStyleClass("is-invalid Wt-invalid");
    view->bindString("totp-code-info", WString::tr("Wt.Auth.totp-code-info-invalid"));
    view->bindString("label", "error has-error");

    if (mfaThrottle() && throttle) {
      auto login = static_cast<WInteractWidget*>(view->resolveWidget("login"));
      updateThrottling(login);
    }
  }
    }
  }
}

