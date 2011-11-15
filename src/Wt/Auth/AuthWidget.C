/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/EnterPasswordFields"
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
#include "Wt/WTemplate"
#include "Wt/WText"

#include "Login"
#include "AuthWidget"
#include "web/Utils.h"

#ifndef WT_DEBUG_JS
#include "js/AuthWidget.min.js"
#endif

namespace skeletons {
  extern const char * Auth_xml1;
}

namespace Wt {
  namespace Auth {

AuthWidget::AuthWidget(const AuthService& baseAuth,
		       AbstractUserDatabase& users, Login& login,
		       WContainerWidget *parent)
  : WTemplate(parent),
    baseAuth_(baseAuth),
    users_(users),
    login_(login),
    passwordAuth_(0),
    registrationEnabled_(false),
    created_(false),
    dialog_(0),
    messageBox_(0),
    enterPasswordFields_(0)
{
  addFunction("id", &WTemplate::Functions::id);
  addFunction("tr", &WTemplate::Functions::tr);

  WApplication *app = WApplication::instance();
  app->useStyleSheet(WApplication::resourcesUrl() + "form.css");
  app->internalPathChanged().connect(this, &AuthWidget::onPathChange);
  app->builtinLocalizedStrings().useBuiltin(skeletons::Auth_xml1);
}

void AuthWidget::addPasswordAuth(const AbstractPasswordService *auth)
{
  passwordAuth_ = auth;
}

void AuthWidget::addOAuth(const OAuthService *auth)
{
  oAuth_.push_back(auth);
}

void AuthWidget::addOAuth(const std::vector<const OAuthService *>& auth)
{
  Utils::insert(oAuth_, auth);
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

      if (ap == "register") {
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

    if (!WApplication::instance()->environment().ajax()) {
      /*
       * try to center it better, we need to set the half width and
       * height as negative margins.
       */
      dialog_->setMargin("-21em", Left); // .Wt-form width
      dialog_->setMargin("-200px", Top); // ???
    }

    dialog_->show();
  }

  return dialog_;
}

void AuthWidget::closeDialog()
{
  if (messageBox_) {
    delete messageBox_;
    messageBox_ = 0;
  } else {
    delete dialog_;
    dialog_ = 0;
  }
}

WWidget *AuthWidget::createRegistrationView(const Identity& id)
{
  RegistrationWidget *w
    = new RegistrationWidget(baseAuth_, users_, login_, this);

  if (passwordAuth_)
    w->model()->addPasswordAuth(passwordAuth_);

  w->model()->addOAuth(oAuth_);

  if (id.isValid())
    w->model()->registerIdentified(id);

  return w;
}

void AuthWidget::handleLostPassword()
{
  showDialog(tr("Wt.Auth.lostpassword"), createLostPasswordView());
}

WWidget *AuthWidget::createLostPasswordView()
{
  return new LostPasswordWidget(users_, baseAuth_);
}

void AuthWidget::letUpdatePassword(const User& user, bool promptPassword)
{
  showDialog(tr("Wt.Auth.updatepassword"),
	     createUpdatePasswordView(user, promptPassword));
}

WWidget *AuthWidget::createUpdatePasswordView(const User& user,
					      bool promptPassword)
{
  return new UpdatePasswordWidget(user, *passwordAuth_, login_, promptPassword);
}

WDialog *AuthWidget::createPasswordPromptDialog(Login& login)
{
  return new PasswordPromptDialog(login, *passwordAuth_);
}

void AuthWidget::logout()
{
  login_.logout();
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

  WTemplate::render(flags);
}

void AuthWidget::create()
{
  if (created_)
    return;

  created_ = true;

  login_.changed().connect(this, &AuthWidget::onLoginChange);
  onLoginChange();
}

void AuthWidget::onLoginChange()
{
  delete enterPasswordFields_;
  enterPasswordFields_ = 0;

  if (login_.loggedIn()) {
    createLoggedInView();
  } else {
    createLoginView();
  }
}

void AuthWidget::createLoginView()
{
  clear();
  setTemplateText(tr("Wt.Auth.template.login"));

  WPushButton *login = new WPushButton(tr("Wt.Auth.login"));
  login->clicked().connect(this, &AuthWidget::attemptLogin);
  bindWidget("login", login);

  createPasswordLogin();
  createOAuthLogin();
}

void AuthWidget::createPasswordLogin()
{
  if (passwordAuth_) {
    setCondition("if:passwords", true);
    WLineEdit *userName = new WLineEdit();
    userName->setFocus();
    bindWidget("user-name", userName);
    WText *userNameInfo = new WText();
    bindWidget("user-name-info", userNameInfo);

    switch (baseAuth_.identityPolicy()) {
    case LoginNameIdentity:
    case OptionalIdentity:
      bindString("user-name-label", tr("Wt.Auth.user-name"));
      userNameInfo->setText(tr("Wt.Auth.user-name-info"));
      break;
    case EmailAddressIdentity:
      bindString("user-name-label", tr("Wt.Auth.email"));
      userNameInfo->setText(tr("Wt.Auth.email-info"));
      break;
    default:
      break;
    }

    WLineEdit *password = new WLineEdit();
    bindWidget("password", password);
    WText *passwordInfo = new WText();
    bindWidget("password-info", passwordInfo);

    if (baseAuth_.authTokensEnabled()) {
      setCondition("if:remember-me", true);
      WCheckBox *rememberme = new WCheckBox();
      bindWidget("remember-me", rememberme);

      int days = baseAuth_.authTokenValidity() / 24 / 60;
      WString info = tr("Wt.Auth.remember-me-info"); 
      if (days % 7 != 0)
	info.arg(boost::lexical_cast<std::string>(days) + " days");
      else
	info.arg(boost::lexical_cast<std::string>(days/7) + " weeks");
      bindString("remember-me-info", info);
    }

    if (baseAuth_.emailVerificationEnabled()) {
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

    if (baseAuth_.emailVerificationEnabled() && registrationEnabled_)
      bindString("sep", " | ");
    else
      bindEmpty("sep");

    WPushButton *login = resolve<WPushButton *>("login");
    enterPasswordFields_
      = new EnterPasswordFields(*passwordAuth_, password, passwordInfo,
				login, this);

    if (baseAuth_.authTokensEnabled()) {
      WApplication::instance()->setCookie
	(baseAuth_.authTokenCookieName(), std::string(), 0);
    }
  }
}

void AuthWidget::createOAuthLogin()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/AuthWidget.js", "authPopupWindow", wtjs1);

  WContainerWidget *icons = new WContainerWidget();
  icons->setInline(isInline());

  for (unsigned i = 0; i < oAuth_.size(); ++i) {
    const OAuthService *auth = oAuth_[i];

    WImage *w = new WImage("css/oauth-" + auth->name() + ".png", icons);
    w->setToolTip(auth->description());
    w->setStyleClass("Wt-auth-icon");
    w->setVerticalAlignment(AlignMiddle);
    OAuthProcess *process = auth->createProcess(auth->authenticationScope());
    w->clicked().connect(process, &OAuthProcess::startAuthenticate);

    process->authenticated().connect
      (boost::bind(&AuthWidget::oAuthDone, this, process, _1));

    WObject::addChild(process);
  }

  if (icons->count() > 0)
    setCondition("if:oauth", true);

  bindWidget("icons", icons);
}

void AuthWidget::oAuthDone(OAuthProcess *oauth, const Identity& identity)
{
  if (identity.isValid()) {
    Wt::log("auth")
      << oauth->service().name() << ": identified: as "
      << identity.id() << ", " << identity.name() << ", " << identity.email();

    std::auto_ptr<AbstractUserDatabase::Transaction>
      t(users_.startTransaction());

    User user = baseAuth_.identifyUser(identity, users_);
    if (user.isValid())
      login_.login(user);
    else
      registerNewUser(identity);

    if (t.get())
      t->commit();
  } else {
    Wt::log("auth") << oauth->service().name() << ": error: " << oauth->error();
    displayError(oauth->error());
  }
}

void AuthWidget::createLoggedInView()
{
  setTemplateText(tr("Wt.Auth.template.logged-in"));

  bindString("user-name", login_.user().identity(Identity::LoginName));  

  WPushButton *logout = new WPushButton(tr("Wt.Auth.logout"));
  logout->clicked().connect(this, &AuthWidget::logout);

  bindWidget("logout", logout);
}

void AuthWidget::attemptLogin()
{
  if (passwordAuth_) {
    WFormWidget *userName = resolve<WFormWidget *>("user-name");
    WText *userNameInfo = resolve<WText *>("user-name-info");

    WFormWidget *rememberme = resolve<WFormWidget *>("remember-me");

    bool remember = rememberme ? rememberme->valueText() == "yes" : 0;

    std::auto_ptr<AbstractUserDatabase::Transaction> t
      (users_.startTransaction());

    User user = users_.findWithIdentity(Identity::LoginName,
					userName->valueText());
    bool valid = user.isValid();
    userName->toggleStyleClass("Wt-invalid", !valid);
    if (userNameInfo) {
      userNameInfo->toggleStyleClass("Wt-error", !valid);
      userNameInfo->setText(valid ? tr("Wt.Auth.valid")
			    : tr("Wt.Auth.user-name-invalid"));
    }

    if (!enterPasswordFields_->validate(user))
      valid = false;

    if (valid) {
      if (remember)
	setAuthToken(user);

      login_.login(user);
    }

    if (t.get())
      t->commit();
  }
}

void AuthWidget::setAuthToken(const User& user)
{
  WApplication *app = WApplication::instance();
  app->setCookie(baseAuth_.authTokenCookieName(),
		 baseAuth_.createAuthToken(user),
		 baseAuth_.authTokenValidity() * 60);
}

void AuthWidget::processEnvironment()
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (registrationEnabled_)
    if (handleRegistrationPath(env.internalPath()))
      return;

  if (baseAuth_.emailVerificationEnabled()) {
    std::string token = baseAuth_.parseEmailToken(env.internalPath());

    if (!token.empty()) {
      EmailTokenResult result = baseAuth_.processEmailToken(token, users_);

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
	login_.login(result.user());
      }

      WApplication::instance()->setInternalPath("/");

      return;
    }
  }

  if (baseAuth_.authTokensEnabled()) {
    std::string token;
    try {
      token = env.getCookie(baseAuth_.authTokenCookieName());
    } catch (...) { }

    if (!token.empty()) {
      AuthTokenResult result = baseAuth_.processAuthToken(token, users_);

      switch (result.result()) {
      case AuthTokenResult::Valid:
	WApplication::instance()
	  ->setCookie(baseAuth_.authTokenCookieName(),
		      result.newToken(),
		      baseAuth_.authTokenValidity() * 60);

	login_.login(result.user(), WeakLogin);
	break;
      case AuthTokenResult::Invalid:
	WApplication::instance()->setCookie(baseAuth_.authTokenCookieName(),
					    std::string(), 0);
	break;
      }
    }
  }
}

  }
}
