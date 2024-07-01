#include "pinwidget.h"

#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Login.h"

#include "Wt/WPushButton.h"
#include "Wt/WWidget.h"

PinWidget::PinWidget(const Wt::Auth::AuthService& authService, Wt::Auth::AbstractUserDatabase& users, Wt::Auth::Login& login)
  : AbstractMfaProcess(authService, users, login)
{
}

std::unique_ptr<Wt::WTemplate> PinWidget::createBaseView(bool isSetup)
{
  auto view = std::make_unique<Wt::WTemplate>(Wt::WTemplate::tr("pin-template"));

  view->addFunction("id", &Wt::WTemplate::Functions::id);
  view->addFunction("tr", &Wt::WTemplate::Functions::tr);

  view->setCondition("if:is-setup", isSetup);

  view_ = view.get();
  return view;
}

std::unique_ptr<Wt::WWidget> PinWidget::createSetupView()
{
  auto view = createBaseView(true);

  createCodeGenerator();
  createCodeInput();
  createLoginButton();
  createRememberMe();
  return std::move(view);
}

std::unique_ptr<Wt::WWidget> PinWidget::createInputView()
{
  auto view = createBaseView(false);

  createCodeInput();
  createLoginButton();
  createRememberMe();
  return std::move(view);
}

void PinWidget::createCodeGenerator()
{
  // Set seed for randomness to current time
  std::srand(std::time(0));

  std::string code;
  int value = 0;
  for (int i = 0; i < NUMBER_OF_DIGITS; ++i) {
    value = std::rand() / (RAND_MAX / 10);
    code += std::to_string(value);
  }

  currentCode_ = code;
  view_->bindString("code", code);
}

void PinWidget::createCodeInput()
{
  codeInput_ = view_->bindNew<Wt::WLineEdit>("input");
  codeInput_->enterPressed().connect([this] {
    checkCodeInput();
  });

  // Validation message
  view_->bindEmpty("code-info");
}

void PinWidget::createLoginButton()
{
  auto login = view_->bindNew<Wt::WPushButton>("login", Wt::WString::tr("Wt.Auth.login"));
  login->clicked().connect([this] {
    checkCodeInput();
  });
}

void PinWidget::createRememberMe()
{
  view_->setCondition("if:remember-me", true);
  rememberMeField_ = view_->bindNew<Wt::WCheckBox>("remember-me");

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

void PinWidget::checkCodeInput()
{
  const std::string enteredCode = codeInput_->text().toUTF8();
  auto savedCode = currentCode_.empty() ? userIdentity() : currentCode_;

  if (enteredCode != savedCode) {
    update();
    authenticated_.emit(Wt::Auth::Mfa::AuthenticationResult(Wt::Auth::Mfa::AuthenticationStatus::Failure, "The validation failed"));
  } else {
    createUserIdentity(savedCode);

    if (rememberMeField_->isChecked()) {
      setRememberMeCookie(login().user());
    }

    login().login(login().user());
    authenticated_.emit(Wt::Auth::Mfa::AuthenticationResult(Wt::Auth::Mfa::AuthenticationStatus::Success));
  }
}

void PinWidget::update()
{
  codeInput_->addStyleClass("is-invalid Wt-invalid");

  // Validation message and color
  view_->bindString("code-info", "Wrong PIN code");
  view_->bindString("label", "invalid-feedback");
}

