/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthModel.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/LostPasswordWidget.h"
#include "Wt/Auth/OAuthWidget.h"
#include "Wt/Auth/PasswordPromptDialog.h"
#include "Wt/Auth/RegistrationWidget.h"
#include "Wt/Auth/UpdatePasswordWidget.h"

#include "Wt/Auth/OAuthService.h"

#include "Wt/WApplication.h"
#include "Wt/WAnchor.h"
#include "Wt/WCheckBox.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WDialog.h"
#include "Wt/WEnvironment.h"
#include "Wt/WImage.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WMessageBox.h"
#include "Wt/WPushButton.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"

#include "Login.h"
#include "AuthWidget.h"
#include "web/WebUtils.h"

#include "Wt/WDllDefs.h"

#include <memory>

namespace Wt {

LOGGER("Auth.AuthWidget");

  namespace Auth {

AuthWidget::AuthWidget(const AuthService& baseAuth,
		       AbstractUserDatabase& users, Login& login)
  : WTemplateFormView(WString::Empty),
    model_(std::make_shared<AuthModel>(baseAuth, users)),
    login_(login)
{
  init();
}

AuthWidget::AuthWidget(Login& login)
  : WTemplateFormView(WString::Empty),
    login_(login)
{ 
  init();
}

AuthWidget::~AuthWidget()
{
  dialog_.reset();
  messageBox_.reset();
}

void AuthWidget::init()
{
  setWidgetIdMode(TemplateWidgetIdMode::SetObjectName);

  registrationEnabled_ = false;
  created_ = false;

  WApplication *app = WApplication::instance();
  app->internalPathChanged().connect(this, &AuthWidget::onPathChange);
  app->theme()->apply(this, this, AuthWidgets);
}

void AuthWidget::setModel(std::unique_ptr<AuthModel> model)
{
  model_ = std::move(model);
}

void AuthWidget::setRegistrationEnabled(bool enabled)
{
  registrationEnabled_ = enabled;
}

void AuthWidget::setInternalBasePath(const std::string& basePath)
{
  basePath_ = Utils::append(Utils::prepend(basePath, '/'), '/');;
}

void AuthWidget::onPathChange(const std::string& path)
{
  handleRegistrationPath(path);
}

bool AuthWidget::handleRegistrationPath(const std::string& path)
{
  if (!basePath_.empty()) {
    WApplication *app = WApplication::instance();

    if (app->internalPathMatches(basePath_)) {
      std::string ap = app->internalSubPath(basePath_);

      if (ap == "register/") {
	registerNewUser();
	return true;
      }
    }
  }

  return false;
}

void AuthWidget::registerNewUser()
{
  registerNewUser(Identity::Invalid);
}

void AuthWidget::registerNewUser(const Identity& oauth)
{
  showDialog(tr("Wt.Auth.registration"), createRegistrationView(oauth));
}

WDialog *AuthWidget::showDialog(const WString& title,
				std::unique_ptr<WWidget> contents)
{
  if (contents) {
    dialog_.reset(new WDialog(title));
    dialog_->contents()->addWidget(std::move(contents));
    dialog_->contents()->childrenChanged()
      .connect(this, &AuthWidget::closeDialog);

    dialog_->footer()->hide();

    if (!WApplication::instance()->environment().ajax()) {
      /*
       * try to center it better, we need to set the half width and
       * height as negative margins.
       */
      dialog_->setMargin(WLength("-21em"), Side::Left); // .Wt-form width
      dialog_->setMargin(WLength("-200px"), Side::Top); // ???
    }

    dialog_->show();
  }

  return dialog_.get();
}

void AuthWidget::closeDialog()
{
  if (dialog_)
    dialog_.reset();
  else
    messageBox_.reset();
  
  /* Reset internal path */
  if (!basePath_.empty()) {
    WApplication *app = WApplication::instance();
    if (app->internalPathMatches(basePath_)) {
      std::string ap = app->internalSubPath(basePath_);

      if (ap == "register/") {
        app->setInternalPath(basePath_, false);
      }
    }
  }
}

std::unique_ptr<RegistrationModel> AuthWidget::createRegistrationModel()
{
  auto result = std::unique_ptr<RegistrationModel>
    (new RegistrationModel(*model_->baseAuth(), model_->users(), login_));

  if (model_->passwordAuth())
    result->addPasswordAuth(model_->passwordAuth());

  result->addOAuth(model_->oAuth());
  return result;
}

std::unique_ptr<WWidget> AuthWidget::createRegistrationView(const Identity& id)
{
  auto model = createRegistrationModel();

  if (id.isValid())
    model->registerIdentified(id);

  std::unique_ptr<RegistrationWidget> w(new RegistrationWidget(this));
  w->setModel(std::move(model));
  return std::move(w);
}

void AuthWidget::handleLostPassword()
{
  showDialog(tr("Wt.Auth.lostpassword"), createLostPasswordView());
}

std::unique_ptr<WWidget> AuthWidget::createLostPasswordView()
{
  return std::unique_ptr<WWidget>
    (new LostPasswordWidget(model_->users(), *model_->baseAuth()));
}

void AuthWidget::letUpdatePassword(const User& user, bool promptPassword)
{
  std::unique_ptr<WWidget> updatePasswordView = createUpdatePasswordView(user, promptPassword);

  UpdatePasswordWidget *defaultUpdatePasswordWidget =
      dynamic_cast<UpdatePasswordWidget*>(updatePasswordView.get());

  showDialog(tr("Wt.Auth.updatepassword"), std::move(updatePasswordView));

  if (defaultUpdatePasswordWidget) {
    defaultUpdatePasswordWidget->updated().connect(this, &AuthWidget::closeDialog);
    defaultUpdatePasswordWidget->canceled().connect(this, &AuthWidget::closeDialog);
  }
}

std::unique_ptr<WWidget> AuthWidget
::createUpdatePasswordView(const User& user, bool promptPassword)
{
  return std::unique_ptr<WWidget>
    (new UpdatePasswordWidget
     (user, createRegistrationModel(),
      promptPassword ? model_ : std::shared_ptr<AuthModel>()));
}

WDialog *AuthWidget::createPasswordPromptDialog(Login& login)
{
  return new PasswordPromptDialog(login, model_);
}

void AuthWidget::logout()
{
  model_->logout(login_);
}

void AuthWidget::displayError(const WString& m)
{
  messageBox_.reset(new WMessageBox(tr("Wt.Auth.error"), m, 
				    Icon::None, StandardButton::Ok));
  messageBox_->buttonClicked().connect(this, &AuthWidget::closeDialog);
  messageBox_->show();
}

void AuthWidget::displayInfo(const WString& m)
{
  messageBox_.reset(new WMessageBox(tr("Wt.Auth.notice"), m, 
				    Icon::None, StandardButton::Ok));
  messageBox_->buttonClicked().connect(this, &AuthWidget::closeDialog);
  messageBox_->show();
}

void AuthWidget::render(WFlags<RenderFlag> flags)
{
  if (!created_) {
    create();
    created_ = true;
  }

  WTemplateFormView::render(flags);
}

void AuthWidget::create()
{
  if (created_)
    return;

  login_.changed().connect(this, &AuthWidget::onLoginChange);
  onLoginChange();

  created_ = true;
}

void AuthWidget::onLoginChange()
{
  clear();

  if (login_.loggedIn()) {
#ifndef WT_TARGET_JAVA
    if (created_) // do not do this if onLoginChange() is called from create()
      WApplication::instance()->changeSessionId();
#endif // WT_TARGET_JAVA

    createLoggedInView();
  } else {
    if (login_.state() != LoginState::Disabled) {
      if (model_->baseAuth()->authTokensEnabled()) {
	WApplication::instance()->removeCookie
	  (model_->baseAuth()->authTokenCookieName());
      }
      
      model_->reset();
      createLoginView();
    } else {
	  createLoginView();
	}
  }
}

void AuthWidget::createLoginView()
{
  setTemplateText(tr("Wt.Auth.template.login"));

  createPasswordLoginView();
  createOAuthLoginView();
}

void AuthWidget::createPasswordLoginView()
{
  updatePasswordLoginView();
}

std::unique_ptr<WWidget> AuthWidget::createFormWidget(WFormModel::Field field)
{
  std::unique_ptr<WFormWidget> result;

  if (field == AuthModel::LoginNameField) {
    result.reset(new WLineEdit());
    result->setFocus(true);
  } else if (field == AuthModel::PasswordField) {
    WLineEdit *p = new WLineEdit();
    p->enterPressed().connect(this, &AuthWidget::attemptPasswordLogin);
    p->setEchoMode(EchoMode::Password);
    result.reset(p);
  } else if (field == AuthModel::RememberMeField) {
    result.reset(new WCheckBox());
  }

  return std::move(result);
}

void AuthWidget::updatePasswordLoginView()
{
  if (model_->passwordAuth()) {
    setCondition("if:passwords", true);

    updateView(model_.get());

    WInteractWidget *login = resolve<WInteractWidget *>("login");

    if (!login) {
      login = bindWidget("login", cpp14::make_unique<WPushButton>(tr("Wt.Auth.login")));
      login->clicked().connect(this, &AuthWidget::attemptPasswordLogin);

      model_->configureThrottling(login);

      if (model_->baseAuth()->emailVerificationEnabled()) {
	WText *text =
	  bindWidget("lost-password",
		     cpp14::make_unique<WText>(tr("Wt.Auth.lost-password")));
	text->clicked().connect(this, &AuthWidget::handleLostPassword);
      } else
	bindEmpty("lost-password");

      if (registrationEnabled_) {
	if (!basePath_.empty()) {
	  bindWidget("register",
		     cpp14::make_unique<WAnchor>
		     (WLink(LinkType::InternalPath, basePath_ + "register"),
		      tr("Wt.Auth.register")));
	} else {
	  WText *t = 
	    bindWidget("register",
		       cpp14::make_unique<WText>(tr("Wt.Auth.register")));
	  t->clicked().connect(this, &AuthWidget::registerNewUser);
	}
      } else
	bindEmpty("register");

      if (model_->baseAuth()->emailVerificationEnabled()
	  && registrationEnabled_)
	bindString("sep", " | ");
      else
	bindEmpty("sep");
    }

    model_->updateThrottling(login);
  } else {
    bindEmpty("lost-password");
    bindEmpty("sep");
    bindEmpty("register");
    bindEmpty("login");
  }
}

void AuthWidget::createOAuthLoginView()
{
  if (!model_->oAuth().empty()) {
    setCondition("if:oauth", true);

    WContainerWidget *icons
      = bindWidget("icons", cpp14::make_unique<WContainerWidget>());
    icons->setInline(isInline());

    for (unsigned i = 0; i < model_->oAuth().size(); ++i) {
      const OAuthService *service = model_->oAuth()[i];

      OAuthWidget *w
	= icons->addWidget(cpp14::make_unique<OAuthWidget>(*service));
      w->authenticated().connect(this, &AuthWidget::oAuthDone);
    }
  }
}

void AuthWidget::oAuthDone(OAuthProcess *oauth, const Identity& identity)
{
  /*
   * FIXME: perhaps consider moving this to the model with signals or
   * by passing the Login object ?
   */
  if (identity.isValid()) {
    LOG_SECURE(oauth->service().name() << ": identified: as "
	       << identity.id() << ", "
	       << identity.name() << ", " << identity.email());

    std::unique_ptr<AbstractUserDatabase::Transaction>
      t(model_->users().startTransaction());

    User user = model_->baseAuth()->identifyUser(identity, model_->users());
    if (user.isValid())
      model_->loginUser(login_, user);
    else
      registerNewUser(identity);

    if (t.get())
      t->commit();
  } else {
    LOG_SECURE(oauth->service().name() << ": error: " << oauth->error());
    displayError(oauth->error());
  }
}

void AuthWidget::attemptPasswordLogin()
{
  updateModel(model_.get());
 
  if (model_->validate()) {
    if (!model_->login(login_))
      updatePasswordLoginView();
  } else
    updatePasswordLoginView();
}

void AuthWidget::createLoggedInView()
{
  setTemplateText(tr("Wt.Auth.template.logged-in"));

  bindString("user-name", login_.user().identity(Identity::LoginName));  

  WPushButton *logout
    = bindWidget("logout",
		 cpp14::make_unique<WPushButton>(tr("Wt.Auth.logout")));
  logout->clicked().connect(this, &AuthWidget::logout);
}

void AuthWidget::processEnvironment()
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (registrationEnabled_)
    if (handleRegistrationPath(env.internalPath()))
      return;

  std::string emailToken
    = model_->baseAuth()->parseEmailToken(env.internalPath());

  if (!emailToken.empty()) {
    EmailTokenResult result = model_->processEmailToken(emailToken);
    switch (result.state()) {
    case EmailTokenState::Invalid:
      displayError(tr("Wt.Auth.error-invalid-token"));
      break;
    case EmailTokenState::Expired:
      displayError(tr("Wt.Auth.error-token-expired"));
      break;
    case EmailTokenState::UpdatePassword:
      letUpdatePassword(result.user(), false);
      break;
    case EmailTokenState::EmailConfirmed:
      displayInfo(tr("Wt.Auth.info-email-confirmed"));
      User user = result.user();
      model_->loginUser(login_, user);
    }

    /*
     * In progressive bootstrap mode, this would cause a redirect w/o
     * session ID, losing the dialog.
     */
    if (WApplication::instance()->environment().ajax())
      WApplication::instance()->setInternalPath("/");

    return;
  }

  User user = model_->processAuthToken();
  model_->loginUser(login_, user, LoginState::Weak);
}

  }
}
