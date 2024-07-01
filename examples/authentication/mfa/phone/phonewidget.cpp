#include "phonewidget.h"

#include "qrcodepainter.h"

#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Login.h"

#include "Wt/WApplication.h"
#include "Wt/WDialog.h"
#include "Wt/WPushButton.h"
#include "Wt/WWidget.h"

PhoneWidget::PhoneWidget(MySession& session)
  : AbstractMfaProcess(session.auth(), session.users(), session.login()),
    session_(session)
{
}

void PhoneWidget::createQRHandlerResource()
{
  qrHandler_ = std::make_unique<QrCodeHandler>(session_, this);
  qrHandler_->allowSignin().connect([this] {
    Wt::log("info") << "createQRHandlerResource(): Performing QR Handler signin response.";
    {
      auto app = Wt::WApplication::instance();
      Wt::WApplication::UpdateLock lock(app);
      login().login(login().user());
      app->triggerUpdate();
    }

    authenticated_.emit(Wt::Auth::Mfa::AuthenticationResult(Wt::Auth::Mfa::AuthenticationStatus::Success));
    if (doRememberMe_) {
      Wt::log("info") << "createQRHandlerResource(): Setting remember me cookie.";
      setRememberMeCookie(login().user());
    }
  });
}

std::unique_ptr<Wt::WTemplate> PhoneWidget::createBaseView(bool isSetup)
{
  auto view = std::make_unique<Wt::WTemplate>(Wt::WTemplate::tr("phone-template"));

  view->addFunction("id", &Wt::WTemplate::Functions::id);
  view->addFunction("tr", &Wt::WTemplate::Functions::tr);

  view_ = view.get();
  return view;
}

std::unique_ptr<Wt::WWidget> PhoneWidget::createSetupView()
{
  auto view = createBaseView(true);

  createQRCode(true);
  createRememberMe();
  return std::move(view);
}

std::unique_ptr<Wt::WWidget> PhoneWidget::createInputView()
{
  auto view = createBaseView(false);

  createQRCode(false);
  createRememberMe();
  return std::move(view);
}

void PhoneWidget::processEnvironment()
{
  Wt::log("info") << "processEnvironment(): Checking environment for valid tokens.";
  Wt::Auth::User user = processMfaToken();

  if (user.isValid()) {
    login().login(user, Wt::Auth::LoginState::Weak);
    authenticated_.emit(Wt::Auth::Mfa::AuthenticationResult(Wt::Auth::Mfa::AuthenticationStatus::Success));
    Wt::log("info") << "processEnvironment(): Found valid token.";
    return;
  }
  Wt::log("info") << "processEnvironment(): Found NO valid token.";
}

void PhoneWidget::setUpUserIdentity()
{
  createUserIdentity("verified");
}

void PhoneWidget::createQRCode(bool isSetup)
{
  createQRHandlerResource();
  std::string handleUrl = Wt::WApplication::instance()->makeAbsoluteUrl(qrHandler_->url());
  if (isSetup) {
    handleUrl += "&issetup";
  }
  view_->bindNew<QrCodePainter>("code", handleUrl);
}

void PhoneWidget::createRememberMe()
{
  view_->setCondition("if:remember-me", true);
  auto rememberMe  = view_->bindNew<Wt::WCheckBox>("remember-me");
  rememberMe->changed().connect([this, rememberMe] { doRememberMe_ = rememberMe->isChecked(); });

  int days = baseAuth().mfaTokenValidity() / 24 / 60;

  Wt::WString info;
  if (days % 7 != 0) {
    info = Wt::WString::trn("Wt.Auth.remember-me-info.days", days).arg(days);
  } else if (days == 0) {
    info = Wt::WString::tr("Wt.Auth.remember-me-info.indefinite");
  } else {
    info = Wt::WString::trn("Wt.Auth.remember-me-info.weeks", days/7).arg(days/7);
  }

  view_->bindString("remember-me-info", info);
}

