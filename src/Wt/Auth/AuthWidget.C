/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordAuth"
#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/BaseAuth"
#include "Wt/Auth/EnterPasswordFields"
#include "Wt/Auth/LostPasswordWidget"
#include "Wt/Auth/PasswordPromptDialog"
#include "Wt/Auth/RegistrationWidget"
#include "Wt/Auth/UpdatePasswordWidget"

#include "Wt/Auth/OAuth"

#include "Wt/WApplication"
#include "Wt/WAnchor"
#include "Wt/WCheckBox"
#include "Wt/WContainerWidget"
#include "Wt/WDialog"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WLineEdit"
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

const char *AuthWidget::PasswordLoginWidget = "passwords";
const char *AuthWidget::OAuthLoginWidget = "oauth";

AuthWidget::AuthWidget(const BaseAuth& baseAuth,
		       AbstractUserDatabase& users, Login& login,
		       WContainerWidget *parent)
  : WCompositeWidget(parent),
    baseAuth_(baseAuth),
    users_(users),
    login_(login),
    passwordAuth_(0),
    registrationEnabled_(false),
    created_(false),
    dialog_(0),
    enterPasswordFields_(0)
{
  setImplementation(impl_ = new WContainerWidget());

  WApplication *app = WApplication::instance();
  app->useStyleSheet(WApplication::resourcesUrl() + "form.css");
  app->internalPathChanged().connect(this, &AuthWidget::onPathChange);
  app->builtinLocalizedStrings().useBuiltin(skeletons::Auth_xml1);
}

void AuthWidget::addPasswordAuth(const AbstractPasswordAuth *auth)
{
  passwordAuth_ = auth;
}

void AuthWidget::addOAuth(const OAuth *auth)
{
  oAuth_.push_back(auth);
}

void AuthWidget::addOAuth(const std::vector<const OAuth *>& auth)
{
  Utils::insert(oAuth_, auth);
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
    dialog_->show();
  }

  return dialog_;
}

void AuthWidget::closeDialog()
{
  delete dialog_;
  dialog_ = 0;
}

WWidget *AuthWidget::createRegistrationView(const Identity& id)
{
  RegistrationWidget *w
    = new RegistrationWidget(baseAuth_, users_, login_, this);

  if (passwordAuth_)
    w->addPasswordAuth(passwordAuth_);

  w->addOAuth(oAuth_);

  if (id.isValid())
    w->registerIdentified(id);

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
  return new UpdatePasswordWidget(user, *passwordAuth_, promptPassword);
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
  // FIXME
}

void AuthWidget::displayInfo(const WString& m)
{
  // FIXME
}

void AuthWidget::render(WFlags<RenderFlag> flags)
{
  if (!created_) {
    create();
    created_ = true;
  }

  WCompositeWidget::render(flags);
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
  impl_->clear();
  delete enterPasswordFields_;
  enterPasswordFields_ = 0;

  if (login_.loggedIn()) {
    impl_->addWidget(createLoggedInView());
  } else {
    impl_->addWidget(createLoginView());
  }
}

WWidget *AuthWidget::createLoginView()
{
  WContainerWidget *result = new WContainerWidget();
  result->setInline(isInline());

  if (passwordAuth_)
    result->addWidget(createPasswordLogin());

  if (!oAuth_.empty())
    result->addWidget(createOAuthLogin());

  return result;
}

WWidget *AuthWidget::createPasswordLogin()
{
  WTemplate *t = new WTemplate(tr("Wt.Auth.template.passwords-login"));
  t->bindFunction("id", &WTemplate::Functions::id);
  t->bindFunction("tr", &WTemplate::Functions::tr);
  t->setInline(isInline());
  t->setObjectName(PasswordLoginWidget);

  WLineEdit *name = new WLineEdit();
  t->bindWidget("name", name);

  WLineEdit *password = new WLineEdit();
  t->bindWidget("password", password);

  if (baseAuth_.authTokensEnabled()) {
    WCheckBox *rememberme = new WCheckBox(tr("Wt.Auth.remember-me"));
    t->bindWidget("remember-me", rememberme);
  } else
    t->bindEmpty("remember-me");

  if (baseAuth_.emailVerificationEnabled()) {
    WText *text = new WText(tr("Wt.Auth.lost-password"));
    text->clicked().connect(this, &AuthWidget::handleLostPassword);
    t->bindWidget("lost-password", text);
  } else
    t->bindEmpty("lost-password");

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

    t->bindWidget("register", w);
  } else
    t->bindEmpty("register");

  WPushButton *login = new WPushButton(tr("Wt.Auth.login"));
  login->clicked().connect(this, &AuthWidget::attemptLogin);
  t->bindWidget("login", login);

  enterPasswordFields_
    = new EnterPasswordFields(*passwordAuth_, password, 0, login, this);

  if (baseAuth_.authTokensEnabled()) {
    WApplication::instance()->setCookie
      (baseAuth_.authTokenCookieName(), std::string(), 0);
  }

  return t;
}

WWidget *AuthWidget::createOAuthLogin()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/AuthWidget.js", "authPopupWindow", wtjs1);

  WTemplate *t = new WTemplate(tr("Wt.Auth.template.oauth-login"));
  t->bindFunction("id", &WTemplate::Functions::id);
  t->bindFunction("tr", &WTemplate::Functions::tr);

  t->setInline(isInline());
  t->setObjectName(OAuthLoginWidget);

  WContainerWidget *icons = new WContainerWidget();
  icons->setInline(isInline());

  for (unsigned i = 0; i < oAuth_.size(); ++i) {
    const OAuth *auth = oAuth_[i];

    WImage *w = new WImage(auth->icon(), icons);
    w->setToolTip(auth->description());
    w->setVerticalAlignment(AlignMiddle);
    OAuth::Process *process = auth->createProcess(auth->authenticationScope());
    w->clicked().connect(process, &OAuth::Process::startAuthenticate);

    process->authenticated().connect
      (boost::bind(&AuthWidget::oAuthDone, this, process, _1));

    WObject::addChild(process);
  }

  t->bindWidget("icons", icons);

  return t;
}

void AuthWidget::oAuthDone(OAuth::Process *oauth, const Identity& identity)
{
  if (identity.isValid()) {
    Wt::log("auth")
      << oauth->auth().name() << ": identified: as "
      << identity.id() << ", " << identity.name() << ", " << identity.email();

    User user = users_.findOAuthId(identity.id());
    if (user.isValid()) {
      login_.login(user);
    } else
      registerNewUser(identity);
  } else
    Wt::log("auth") << oauth->auth().name() << ": error: " << oauth->error();
}

WWidget *AuthWidget::createLoggedInView()
{
  WTemplate *t = new WTemplate(tr("Wt.Auth.template.logged-in"), impl_);
  //t->setIdPrefix("id:"); t->setTrPrefix("tr:");
  t->setInline(isInline());

  t->bindString("identity", login_.user().identity());  

  WPushButton *logout = new WPushButton(tr("Wt.Auth.logout"));
  logout->clicked().connect(this, &AuthWidget::logout);

  t->bindWidget("logout", logout);

  return t;
}

void AuthWidget::attemptLogin()
{
  if (passwordAuth_) {
    WTemplate *t = dynamic_cast<WTemplate *>(find("passwords"));

    WFormWidget *name = t->resolve<WFormWidget *>("name");
    WFormWidget *rememberme = t->resolve<WFormWidget *>("remember-me");

    bool remember = rememberme ? rememberme->valueText() == "yes" : 0;

    std::auto_ptr<AbstractUserDatabase::Transaction> tr
      (users_.startTransaction());

    User user = users_.findIdentity(name->valueText());
    bool valid = user.isValid();
    name->toggleStyleClass("Wt-invalid", !valid);

    if (!enterPasswordFields_->validate(user))
      valid = false;

    if (valid) {
      if (remember)
	setAuthToken(user);

      login_.login(user);
    }

    if (tr.get())
      tr->commit();
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

      // Change the internal path to get rid of the token ?

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
