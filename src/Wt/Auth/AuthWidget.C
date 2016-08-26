/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthModel"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/LostPasswordWidget"
#include "Wt/Auth/PasswordPromptDialog"
#include "Wt/Auth/RegistrationWidget"
#include "Wt/Auth/UpdatePasswordWidget"

#include "Wt/Auth/OAuthService"

#include "Wt/WApplication"
#include "Wt/WAnchor"
#include "Wt/WCheckBox"
#include "Wt/WContainerWidget"
#include "Wt/WDialog"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WLineEdit"
#include "Wt/WMessageBox"
#include "Wt/WPushButton"
#include "Wt/WText"
#include "Wt/WTheme"

#include "Login"
#include "AuthWidget"
#include "web/WebUtils.h"

#include "Wt/WDllDefs.h"

#include <memory>

#ifdef WT_CXX11
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

namespace Wt {

LOGGER("Auth.AuthWidget");

  namespace Auth {

AuthWidget::AuthWidget(const AuthService& baseAuth,
		       AbstractUserDatabase& users, Login& login,
		       WContainerWidget *parent)
  : WTemplateFormView(WString::Empty, parent),
    model_(new AuthModel(baseAuth, users, this)),
    login_(login)
{
  init();
}

AuthWidget::AuthWidget(Login& login, WContainerWidget *parent)
  : WTemplateFormView(WString::Empty, parent),
    model_(0),
    login_(login)
{ 
  init();
}

void AuthWidget::init()
{
  setWidgetIdMode(SetWidgetObjectName);

  registrationModel_ = 0;
  registrationEnabled_ = false;
  created_ = false;
  dialog_ = 0;
  messageBox_ = 0;

  WApplication *app = WApplication::instance();
  app->internalPathChanged().connect(this, &AuthWidget::onPathChange);
  app->theme()->apply(this, this, AuthWidgets);
}

void AuthWidget::setModel(AuthModel *model)
{
  delete model_;
  model_ = model;
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

WDialog *AuthWidget::showDialog(const WString& title, WWidget *contents)
{
  delete dialog_;
  dialog_ = 0;

  if (contents) {
    dialog_ = new WDialog(title);
    dialog_->contents()->addWidget(contents);
    dialog_->contents()->childrenChanged()
      .connect(this, &AuthWidget::closeDialog);

    dialog_->footer()->hide();

    if (!WApplication::instance()->environment().ajax()) {
      /*
       * try to center it better, we need to set the half width and
       * height as negative margins.
       */
      dialog_->setMargin(WLength("-21em"), Left); // .Wt-form width
      dialog_->setMargin(WLength("-200px"), Top); // ???
    }

    dialog_->show();
  }

  return dialog_;
}

void AuthWidget::closeDialog()
{
  if (dialog_) {
    delete dialog_;
    dialog_ = 0;
  } else {
    delete messageBox_;
    messageBox_ = 0;
  }
  
  /* Reset internal path */
  if(!basePath_.empty()) {
    WApplication *app = WApplication::instance();
    if (app->internalPathMatches(basePath_)) {
      std::string ap = app->internalSubPath(basePath_);

      if (ap == "register/") {
        app->setInternalPath(basePath_, false);
      }
    }
  }
}

RegistrationModel *AuthWidget::registrationModel()
{
  if (!registrationModel_) {
    registrationModel_ = createRegistrationModel();

    if (model_->passwordAuth())
      registrationModel_->addPasswordAuth(model_->passwordAuth());

    registrationModel_->addOAuth(model_->oAuth());
  } else
    registrationModel_->reset();

  return registrationModel_;
}

RegistrationModel *AuthWidget::createRegistrationModel()
{
  RegistrationModel *result
    = new RegistrationModel(*model_->baseAuth(), model_->users(),
			    login_, this);

  if (model_->passwordAuth())
    result->addPasswordAuth(model_->passwordAuth());

  result->addOAuth(model_->oAuth());

  return result;
}

WWidget *AuthWidget::createRegistrationView(const Identity& id)
{
  RegistrationModel *model = registrationModel();

  if (id.isValid())
    model->registerIdentified(id);

  RegistrationWidget *w = new RegistrationWidget(this);
  w->setModel(model);

  return w;
}

void AuthWidget::handleLostPassword()
{
  showDialog(tr("Wt.Auth.lostpassword"), createLostPasswordView());
}

WWidget *AuthWidget::createLostPasswordView()
{
  return new LostPasswordWidget(model_->users(), *model_->baseAuth());
}

void AuthWidget::letUpdatePassword(const User& user, bool promptPassword)
{
  showDialog(tr("Wt.Auth.updatepassword"),
	     createUpdatePasswordView(user, promptPassword));
}

WWidget *AuthWidget::createUpdatePasswordView(const User& user,
					      bool promptPassword)
{
  return new UpdatePasswordWidget(user, registrationModel(),
				  promptPassword ? model_ : 0);
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
  delete messageBox_;

  WMessageBox *box = new WMessageBox(tr("Wt.Auth.error"), m, NoIcon, Ok);
  box->buttonClicked().connect(this, &AuthWidget::closeDialog);
  box->show();

  messageBox_ = box;
}

void AuthWidget::displayInfo(const WString& m)
{
  delete messageBox_;

  WMessageBox *box = new WMessageBox(tr("Wt.Auth.notice"), m, NoIcon, Ok);
  box->buttonClicked().connect(this, &AuthWidget::closeDialog);
  box->show();

  messageBox_ = box;
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
    if (login_.state() != DisabledLogin) {
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

WWidget *AuthWidget::createFormWidget(WFormModel::Field field)
{
  WFormWidget *result = 0;

  if (field == AuthModel::LoginNameField) {
    result = new WLineEdit();
    result->setFocus(true);
  } else if (field == AuthModel::PasswordField) {
    WLineEdit *p = new WLineEdit();
    p->enterPressed().connect(this, &AuthWidget::attemptPasswordLogin);
    p->setEchoMode(WLineEdit::Password);
    result = p;
  } else if (field == AuthModel::RememberMeField) {
    result = new WCheckBox();
  }

  return result;
}

void AuthWidget::updatePasswordLoginView()
{
  if (model_->passwordAuth()) {
    setCondition("if:passwords", true);

    updateView(model_);

    WInteractWidget *login = resolve<WInteractWidget *>("login");

    if (!login) {
      login = new WPushButton(tr("Wt.Auth.login"));
      login->clicked().connect(this, &AuthWidget::attemptPasswordLogin);
      bindWidget("login", login);

      model_->configureThrottling(login);

      if (model_->baseAuth()->emailVerificationEnabled()) {
	WText *text = new WText(tr("Wt.Auth.lost-password"));
	text->clicked().connect(this, &AuthWidget::handleLostPassword);
	bindWidget("lost-password", text);
      } else
	bindEmpty("lost-password");

      if (registrationEnabled_) {
	WInteractWidget *w;
	if (!basePath_.empty()) {
	  w = new WAnchor
	    (WLink(WLink::InternalPath, basePath_ + "register"),
	     tr("Wt.Auth.register"));
	} else {
	  w = new WText(tr("Wt.Auth.register"));
	  w->clicked().connect(this, &AuthWidget::registerNewUser);
	}

	bindWidget("register", w);
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

    WContainerWidget *icons = new WContainerWidget();
    icons->setInline(isInline());

    for (unsigned i = 0; i < model_->oAuth().size(); ++i) {
      const OAuthService *auth = model_->oAuth()[i];

      WImage *w = new WImage("css/oauth-" + auth->name() + ".png", icons);
      w->setToolTip(auth->description());
      w->setStyleClass("Wt-auth-icon");
      w->setVerticalAlignment(AlignMiddle);

      OAuthProcess *const process 
	= auth->createProcess(auth->authenticationScope());
#ifndef WT_TARGET_JAVA
      w->clicked().connect(process, &OAuthProcess::startAuthenticate);
#else
      process->connectStartAuthenticate(w->clicked());
#endif

      process->authenticated().connect
	(boost::bind(&AuthWidget::oAuthDone, this, process, _1));

      WObject::addChild(process);
    }

    bindWidget("icons", icons);
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

    AUTO_PTR<AbstractUserDatabase::Transaction>
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
  updateModel(model_);
 
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

  WPushButton *logout = new WPushButton(tr("Wt.Auth.logout"));
  logout->clicked().connect(this, &AuthWidget::logout);

  bindWidget("logout", logout);
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
    switch (result.result()) {
    case EmailTokenResult::Invalid:
      displayError(tr("Wt.Auth.error-invalid-token"));
      break;
    case EmailTokenResult::Expired:
      displayError(tr("Wt.Auth.error-token-expired"));
      break;
    case EmailTokenResult::UpdatePassword:
      letUpdatePassword(result.user(), false);
      break;
    case EmailTokenResult::EmailConfirmed:
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
  model_->loginUser(login_, user, WeakLogin);
}

  }
}
