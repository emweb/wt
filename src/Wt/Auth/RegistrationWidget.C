/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthWidget"
#include "Wt/Auth/Login"
#include "Wt/Auth/RegistrationWidget"

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WDialog"
#include "Wt/WImage"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"
#include "Wt/WText"
#include "Wt/WTheme"

#include "Wt/WDllDefs.h"

#include <memory>

#ifdef WT_CXX11
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

namespace Wt {

LOGGER("Auth.RegistrationWidget");	     

  namespace Auth {

RegistrationWidget::RegistrationWidget(AuthWidget *authWidget)
  : WTemplateFormView(tr("Wt.Auth.template.registration")),
    authWidget_(authWidget),
    model_(0),
    created_(false),
    confirmPasswordLogin_(0)
{
  setWidgetIdMode(SetWidgetObjectName);

  WApplication *app = WApplication::instance();
  app->theme()->apply(this, this, AuthWidgets);
}

void RegistrationWidget::setModel(RegistrationModel *model)
{
  if (!model_ && model)
    model->login().changed().connect(this, &RegistrationWidget::close);

  delete model_;
  model_ = model;
}

void RegistrationWidget::render(WFlags<RenderFlag> flags)
{
  if (!created_) {
    update();
    created_ = true;
  }

  WTemplateFormView::render(flags);
}

WWidget *RegistrationWidget::createFormWidget(WFormModel::Field field)
{
  WFormWidget *result = 0;

  if (field == RegistrationModel::LoginNameField) {
    result = new WLineEdit();
    result->changed().connect
      (boost::bind(&RegistrationWidget::checkLoginName, this));
  } else if (field == RegistrationModel::EmailField) {
    result = new WLineEdit();
  } else if (field == RegistrationModel::ChoosePasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(WLineEdit::Password);
    p->keyWentUp().connect
      (boost::bind(&RegistrationWidget::checkPassword, this));
    p->changed().connect
      (boost::bind(&RegistrationWidget::checkPassword, this));
    result = p;
  } else if (field == RegistrationModel::RepeatPasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(WLineEdit::Password);
    p->changed().connect
      (boost::bind(&RegistrationWidget::checkPassword2, this));
    result = p;
  }

  return result;
}

void RegistrationWidget::update()
{
  if (model_->passwordAuth())
    bindString("password-description",
	       tr("Wt.Auth.password-registration"));
  else
    bindEmpty("password-description");

  updateView(model_);

  if (!created_) {
    WLineEdit *password = resolve<WLineEdit *>
      (RegistrationModel::ChoosePasswordField);
    WLineEdit *password2 = resolve<WLineEdit *>
      (RegistrationModel::RepeatPasswordField);
    WText *password2Info = resolve<WText *>
      (RegistrationModel::RepeatPasswordField + std::string("-info"));

    if (password && password2 && password2Info)
      model_->validatePasswordsMatchJS(password, password2, password2Info);
  }

  WAnchor *isYou = resolve<WAnchor *>("confirm-is-you");
  if (!isYou) {
    isYou = new WAnchor(std::string("#"), tr("Wt.Auth.confirm-is-you"));
    isYou->hide();
    bindWidget("confirm-is-you", isYou);
  }

  if (model_->isConfirmUserButtonVisible()) {
    if (!isYou->clicked().isConnected())
      isYou->clicked().connect(this, &RegistrationWidget::confirmIsYou);
    isYou->show();
  } else
    isYou->hide();

  if (model_->isFederatedLoginVisible()) {
    if (!conditionValue("if:oauth")) {
      setCondition("if:oauth", true);
      if (model_->passwordAuth())
	bindString("oauth-description", tr("Wt.Auth.or-oauth-registration"));
      else
	bindString("oauth-description", tr("Wt.Auth.oauth-registration"));

      WContainerWidget *icons = new WContainerWidget();
      icons->addStyleClass("Wt-field");

      for (unsigned i = 0; i < model_->oAuth().size(); ++i) {
	const OAuthService *service = model_->oAuth()[i];

	WImage *w = new WImage("css/oauth-" + service->name() + ".png", icons);
	w->setToolTip(service->description());
	w->setStyleClass("Wt-auth-icon");
	w->setVerticalAlignment(AlignMiddle);
	OAuthProcess *const process
	  = service->createProcess(service->authenticationScope());
	w->clicked().connect(process, &OAuthProcess::startAuthenticate);

	process->authenticated().connect
	  (boost::bind(&RegistrationWidget::oAuthDone, this, process, _1));

	WObject::addChild(process);
      }

      bindWidget("icons", icons);
    }
  } else {
    setCondition("if:oauth", false);
    bindEmpty("icons");
  }

  if (!created_) {
    WPushButton *okButton = new WPushButton(tr("Wt.Auth.register"));
    WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

    bindWidget("ok-button", okButton);
    bindWidget("cancel-button", cancelButton);

    okButton->clicked().connect(this, &RegistrationWidget::doRegister);
    cancelButton->clicked().connect(this, &RegistrationWidget::close);

    created_ = true;
  }
}

void RegistrationWidget::oAuthDone(OAuthProcess *oauth,
				   const Identity& identity)
{
  if (identity.isValid()) {
    LOG_SECURE(oauth->service().name() << ": identified: as "
	       << identity.id() << ", " << identity.name() << ", "
	       << identity.email());

    if (!model_->registerIdentified(identity))
      update();
  } else {
    if (authWidget_)
      authWidget_->displayError(oauth->error());
    LOG_SECURE(oauth->service().name() << ": error: " << oauth->error());
  }
}

void RegistrationWidget::checkLoginName()
{
  updateModelField(model_, RegistrationModel::LoginNameField);
  model_->validateField(RegistrationModel::LoginNameField);
  model_->setValidated(RegistrationModel::LoginNameField, false);
  update();
}

void RegistrationWidget::checkPassword()
{
  updateModelField(model_, RegistrationModel::LoginNameField);
  updateModelField(model_, RegistrationModel::ChoosePasswordField);
  updateModelField(model_, RegistrationModel::EmailField);
  model_->validateField(RegistrationModel::ChoosePasswordField);
  model_->setValidated(RegistrationModel::ChoosePasswordField, false);
  update();
}

void RegistrationWidget::checkPassword2()
{
  updateModelField(model_, RegistrationModel::ChoosePasswordField);
  updateModelField(model_, RegistrationModel::RepeatPasswordField);
  model_->validateField(RegistrationModel::RepeatPasswordField);
  model_->setValidated(RegistrationModel::RepeatPasswordField, false);
  update();
}

bool RegistrationWidget::validate()
{
  return model_->validate();
}

void RegistrationWidget::doRegister()
{
  AUTO_PTR<AbstractUserDatabase::Transaction>
    t(model_->users().startTransaction());

  updateModel(model_);

  if (validate()) {
    User user = model_->doRegister();
    if (user.isValid()) {
      registerUserDetails(user);
      if (!model_->baseAuth()->emailVerificationRequired())
	model_->loginUser(model_->login(), user);
      else {
	if (authWidget_)
	  authWidget_->displayInfo
	    (WString::tr("Wt.Auth.confirm-email-first"));

	close();
      }
    } else
      update();
  } else
    update();

  if (t.get())
    t->commit();
}

void RegistrationWidget::registerUserDetails(User& user)
{ }

void RegistrationWidget::close()
{
  delete this;
}

void RegistrationWidget::confirmIsYou()
{
  updateModel(model_);

  switch (model_->confirmIsExistingUser()) {
  case RegistrationModel::ConfirmWithPassword:
    {
      delete confirmPasswordLogin_;
      confirmPasswordLogin_ = new Login();
      confirmPasswordLogin_->login(model_->existingUser(), WeakLogin);
      confirmPasswordLogin_
	->changed().connect(this, &RegistrationWidget::confirmedIsYou);

      WDialog *dialog =
	authWidget_->createPasswordPromptDialog(*confirmPasswordLogin_);
      dialog->show();
    }

    break;
  case RegistrationModel::ConfirmWithEmail:
    // FIXME send a confirmation email to merge the new identity
    // with the existing one. We need to include the provisional
    // id in the token -- no problem there, integrity is verified by a
    // hash in the database

    LOG_INFO("confirming a new identity to existing user not yet implemented");

    break;
  default:
    LOG_ERROR("that's gone haywire.");
  }
}

void RegistrationWidget::confirmedIsYou()
{
  if (confirmPasswordLogin_->state() == StrongLogin) {
    model_->existingUserConfirmed();
  } else {
    delete confirmPasswordLogin_;
    confirmPasswordLogin_ = 0;
  }
}

  }
}
