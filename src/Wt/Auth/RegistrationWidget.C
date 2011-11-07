/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthWidget"
#include "Wt/Auth/BaseAuth"
#include "Wt/Auth/ChoosePasswordFields"
#include "Wt/Auth/Login"
#include "Wt/Auth/AbstractPasswordAuth"
#include "Wt/Auth/RegistrationWidget"

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WDialog"
#include "Wt/WImage"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WText"

#include "web/Utils.h"

namespace skeletons {
  extern const char *Auth_xml1;
}

namespace Wt {
  namespace Auth {

RegistrationWidget::RegistrationWidget(const BaseAuth& baseAuth,
				       AbstractUserDatabase& users,
				       Login& login,
				       AuthWidget *authWidget)
  : WTemplate(tr("Wt.Auth.template.registration")),
    baseAuth_(baseAuth),
    users_(users),
    login_(login),
    authWidget_(authWidget),
    identityPolicy_(ServiceProvided),
    passwordAuth_(0),
    created_(false),
    passwordFields_(0),
    confirmPasswordLogin_(0)
{
  bindFunction("id", &WTemplate::Functions::id);
  bindFunction("tr", &WTemplate::Functions::tr);

  WApplication *app = WApplication::instance();
  app->builtinLocalizedStrings().useBuiltin(skeletons::Auth_xml1);
}

void RegistrationWidget::addPasswordAuth(const AbstractPasswordAuth *auth)
{
  passwordAuth_ = auth;
}

void RegistrationWidget::addOAuth(const OAuth *auth)
{
  oAuth_.push_back(auth);
}

void RegistrationWidget::addOAuth(const std::vector<const OAuth *>& auth)
{
  Utils::insert(oAuth_, auth);
}

void RegistrationWidget::setIdentityPolicy(IdentityPolicy policy)
{
  identityPolicy_ = policy;
}

void RegistrationWidget::registerIdentified(const Identity& identity)
{
  oAuthIdentity_ = identity;
}

void RegistrationWidget::render(WFlags<RenderFlag> flags)
{
  if (!created_) {
    create();
    created_ = true;
  }

  WTemplate::render(flags);
}

void RegistrationWidget::create()
{
  if (created_)
    return;

  created_ = true;

  createPasswordRegistration();
  createOAuthRegistration();

  WPushButton *okButton = new WPushButton(tr("Wt.Auth.register"));
  WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

  bindWidget("ok-button", okButton);
  bindWidget("cancel-button", cancelButton);

  okButton->clicked().connect(this, &RegistrationWidget::doRegister);
  cancelButton->clicked().connect(this, &RegistrationWidget::close);

  if (oAuthIdentity_.isValid())
    handleOAuthIdentified();
}

void RegistrationWidget::createPasswordRegistration()
{
  if (passwordAuth_ && !oAuth_.empty())
    bindString("password-description",
	       tr("Wt.Auth.password-or-oauth-registration"));
  else
    bindEmpty("password-description");
    
  if (passwordAuth_) {
    addIdentityField();

    WLineEdit *passwd = new WLineEdit();
    WText *passwdInfo = new WText();
    WLineEdit *passwd2 = new WLineEdit();
    WText *passwd2Info = new WText();

    passwordFields_ 
      = new ChoosePasswordFields(*passwordAuth_,
				 passwd, passwdInfo, passwd2, passwd2Info,
				 this);

    bindString("password-display", "");
    bindWidget("password", passwd);
    bindWidget("password-info", passwdInfo);

    bindWidget("password2", passwd2);
    bindWidget("password2-info", passwd2Info);
  }

  if (baseAuth_.emailVerificationEnabled()) {
    WLineEdit *email = new WLineEdit();
    bindWidget("email", email);
    bindString("email-display", "");
  } else
    bindString("email-display", "none");
}

void RegistrationWidget::addIdentityField()
{
  if (!resolveWidget("identity")) {
    WLineEdit *identity = new WLineEdit();
    WText *identityInfo = new WText();
    identityInfo->setText(baseAuth_.validateIdentity(WString::Empty));

    identity->changed().connect
      (boost::bind(&RegistrationWidget::checkIdentity, this));

    bindString("identity-display", "");
    bindWidget("identity", identity);
    bindWidget("identity-info", identityInfo);
  }
}

void RegistrationWidget::createOAuthRegistration()
{
  if (identityPolicy_ == UserChosen)
    addIdentityField();

  if (!oAuth_.empty()) {
    bindString("oauth-display", "");
    if (passwordAuth_)
      bindString("oauth-description", tr("Wt.Auth.or-oauth-registration"));
    else
      bindString("oauth-description", tr("Wt.Auth.oauth-registration"));
  } else
    bindString("oauth-display", "none");

  WContainerWidget *icons = new WContainerWidget();
  icons->addStyleClass("Wt-field");

  for (unsigned i = 0; i < oAuth_.size(); ++i) {
    const OAuth *auth = oAuth_[i];

    WImage *w = new WImage(auth->icon(), icons);
    w->setToolTip(auth->description());
    w->setVerticalAlignment(AlignMiddle);
    OAuth::Process *process = auth->createProcess(auth->authenticationScope());
    w->clicked().connect(process, &OAuth::Process::startAuthenticate);

    process->authenticated().connect
      (boost::bind(&RegistrationWidget::oAuthDone, this, process, _1));

    WObject::addChild(process);
  }

  bindWidget("icons", icons);

  bindEmpty("confirm-is-you");
}

void RegistrationWidget::oAuthDone(OAuth::Process *oauth,
				   const Identity& identity)
{
  if (identity.isValid()) {
    Wt::log("auth")
      << oauth->auth().name() << ": identified: as "
      << identity.id() << ", " << identity.name() << ", " << identity.email();

    oAuthIdentity_ = identity;
    handleOAuthIdentified();
  } else {
    Wt::log("auth") << oauth->auth().name() << ": error: " << oauth->error();
  }
}

void RegistrationWidget::handleOAuthIdentified()
{
  User user = users_.findOAuthId(oAuthIdentity_.id());

  if (user.isValid()) {
    login_.login(user);
    close();
    return;
  }

  WLineEdit *identity = resolve<WLineEdit *>("identity");

  if (identityPolicy_ == UserChosen) {
    WAnchor *isYou = new WAnchor("#", tr("Wt.Auth.confirm-is-you"));
    isYou->hide();
    isYou->clicked().connect(this, &RegistrationWidget::confirmIsYou);
    bindWidget("confirm-is-you", isYou);

    if (identity->text().empty())
      suggestIdentity();
  } else {
    bindString("identity-display", "none");
  }

  delete passwordFields_;
  passwordFields_ = 0;

  bindEmpty("password-description");
  bindString("password-display", "none");
  bindWidget("password", 0);
  bindWidget("password2", 0);

  if (!oAuthIdentity_.email().empty()) {
    WLineEdit *email = resolve<WLineEdit *>("email");

    if (email) {
      email->setText(oAuthIdentity_.email());
      email->disable();
      email->addStyleClass("Wt-disabled");
    }
  }

  WContainerWidget *icons = resolve<WContainerWidget *>("icons");
  icons->clear();

  bindString("oauth-display", "none");
}

void RegistrationWidget::suggestIdentity()
{
  WLineEdit *identity = resolve<WLineEdit *>("identity");

  if (!oAuthIdentity_.email().empty()) {
    std::string suggested = oAuthIdentity_.email();
    std::size_t i = suggested.find('@');
    if (i != std::string::npos)
      suggested = suggested.substr(0, i);

    identity->setText(WString::fromUTF8(suggested));
  } else if (!oAuthIdentity_.name().empty())
    identity->setText(oAuthIdentity_.name());

  if (!identity->text().empty())
    checkIdentity();
}

bool RegistrationWidget::checkIdentity()
{
  WFormWidget *identity = resolve<WFormWidget *>("identity");
  WText *error = resolve<WText *>("identity-info");

  bool valid = true;

  if (identity && error) {
    WString id = identity->valueText();

    WString e = baseAuth_.validateIdentity(id);
    valid = e.empty();

    if (valid)
      error->setText(tr("Wt.Auth.identity-valid"));
    else
      error->setText(e);

    error->toggleStyleClass("Wt-error", !valid);

    if (!valid)
      identity->removeStyleClass("Wt-valid");
    identity->toggleStyleClass("Wt-invalid", !valid);

    if (valid)
      valid = checkIdentityExists();
  }

  return valid;
}

bool RegistrationWidget::checkIdentityExists()
{
  WFormWidget *identity = resolve<WFormWidget *>("identity");
  WAnchor *isYou = resolve<WAnchor *>("confirm-is-you");
  WText *error = resolve<WText *>("identity-info");

  WString id = identity->valueText();
  User u = users_.findIdentity(id);
  bool exists = u.isValid();

  if (exists) {
    if (isYou)
      isYou->show();
    else {
      error->setText(tr("Wt.Auth.identity-exists"));
      error->addStyleClass("Wt-error");
    }
  } else {
    if (isYou)
      isYou->hide();
  }

  identity->toggleStyleClass("Wt-invalid", exists);
  identity->toggleStyleClass("Wt-valid", !exists);

  return !exists;
}

void RegistrationWidget::confirmIsYou()
{
  WFormWidget *identity = resolve<WFormWidget *>("identity");

  WString id = identity->valueText();
  User user = users_.findIdentity(id);
  bool exists = user.isValid();

  if (exists) {
    delete confirmPasswordLogin_;
    confirmPasswordLogin_ = new Login();
    confirmPasswordLogin_->login(user, WeakLogin);
    confirmPasswordLogin_
      ->changed().connect(this, &RegistrationWidget::confirmedIsYou);

    WDialog *dialog =
      authWidget_->createPasswordPromptDialog(*confirmPasswordLogin_);
    dialog->show();
  }
}

void RegistrationWidget::confirmedIsYou()
{
  if (confirmPasswordLogin_->state() == StrongLogin) {
    User user = confirmPasswordLogin_->user();
    delete confirmPasswordLogin_;
    confirmPasswordLogin_ = 0;

    user.addOAuthId(oAuthIdentity_.id());

    login_.login(user);

    close();
  } else {
    delete confirmPasswordLogin_;
    confirmPasswordLogin_ = 0;
  }
}

bool RegistrationWidget::validate()
{
  bool valid = true;

  if (!checkIdentity())
    valid = false;

  if (passwordFields_ && !passwordFields_->validate())
    valid = false;

  return valid;
}

void RegistrationWidget::doRegister()
{
  std::auto_ptr<AbstractUserDatabase::Transaction> t(users_.startTransaction());

  if (validate()) {
    if (!passwordAuth_ && !oAuthIdentity_.isValid()) {
      // FIXME: Set message that you need to identify using oauth.
      return;
    } else {
      WFormWidget *identity = resolve<WFormWidget *>("identity");
      User user;

      if (oAuthIdentity_.isValid()) {
	if (identityPolicy_ == UserChosen)
	  user = users_.registerNew(identity->valueText());
	else
	  user = users_.registerNew(oAuthIdentity_.id());

	if (user.isValid()) {
	  user.addOAuthId(oAuthIdentity_.id());

	  std::string email;
	  bool emailVerified = false;
	  if (!oAuthIdentity_.email().empty()) {
	    email = oAuthIdentity_.email();
	    emailVerified = oAuthIdentity_.emailVerified();
	  } else {
	    WLineEdit *e = resolve<WLineEdit *>("email");
	    if (e)
	      email = e->text().toUTF8();
	  }

	  if (!email.empty()) {
	    if (emailVerified || !baseAuth_.emailVerificationEnabled())
	      user.setEmail(email);
	    else
	      baseAuth_.verifyEmailAddress(user, email);
	  }
	}
      } else {
	WFormWidget *password = resolve<WFormWidget *>("password");

	user = users_.registerNew(identity->valueText());

	if (user.isValid()) {
	  passwordAuth_->updatePassword(user, password->valueText());

	  if (baseAuth_.emailVerificationEnabled()) {
	    WFormWidget *email = resolve<WFormWidget *>("email");
	    baseAuth_.verifyEmailAddress(user, email->valueText().toUTF8());
	  }
	}
      }

      if (user.isValid()) {
	registerUserDetails(user);
	login_.login(user);
	close();
      } else {
	if (identity) {
	  identity->removeStyleClass("Wt-valid");
	  identity->addStyleClass("Wt-invalid");

	  WText *error = resolve<WText *>("identity-info");
	  if (error) {
	    error->setText(tr("Wt.Auth.error-user-invalid"));
	    error->addStyleClass("Wt-error");
	  }
	}
      }
    }
  }

  if (t.get())
    t->commit();
}

void RegistrationWidget::registerUserDetails(User& user)
{ }

void RegistrationWidget::close()
{
  delete this;
}

  }
}
