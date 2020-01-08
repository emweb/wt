/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthWidget.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/OAuthWidget.h"
#include "Wt/Auth/RegistrationWidget.h"

#include "Wt/WAnchor.h"
#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WDialog.h"
#include "Wt/WImage.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WPushButton.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"

#include "Wt/WDllDefs.h"

#include <memory>

namespace Wt {

LOGGER("Auth.RegistrationWidget");	     

  namespace Auth {

RegistrationWidget::RegistrationWidget(AuthWidget *authWidget)
  : WTemplateFormView(tr("Wt.Auth.template.registration")),
    authWidget_(authWidget),
    created_(false)
{
  setWidgetIdMode(TemplateWidgetIdMode::SetObjectName);

  WApplication *app = WApplication::instance();
  app->theme()->apply(this, this, AuthWidgets);
}

void RegistrationWidget::setModel(std::unique_ptr<RegistrationModel> model)
{
  if (!model_ && model)
    model->login().changed().connect(this, &RegistrationWidget::close);

  model_ = std::move(model);
}

void RegistrationWidget::render(WFlags<RenderFlag> flags)
{
  if (!created_) {
    update();
    created_ = true;
  }

  WTemplateFormView::render(flags);
}

std::unique_ptr<WWidget> RegistrationWidget
::createFormWidget(WFormModel::Field field)
{
  std::unique_ptr<WFormWidget> result;

  if (field == RegistrationModel::LoginNameField) {
    result.reset(new WLineEdit());
    result->changed().connect(this, &RegistrationWidget::checkLoginName);
  } else if (field == RegistrationModel::EmailField) {
    result.reset(new WLineEdit());
  } else if (field == RegistrationModel::ChoosePasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(EchoMode::Password);
    p->keyWentUp().connect(this, &RegistrationWidget::checkPassword);
    p->changed().connect(this, &RegistrationWidget::checkPassword);
    result.reset(p);
  } else if (field == RegistrationModel::RepeatPasswordField) {
    WLineEdit *p = new WLineEdit();
    p->setEchoMode(EchoMode::Password);
    p->changed().connect(this, &RegistrationWidget::checkPassword2);
    result.reset(p);
  }

  return std::move(result);
}

void RegistrationWidget::update()
{
  if (model_->passwordAuth())
    bindString("password-description",
	       tr("Wt.Auth.password-registration"));
  else
    bindEmpty("password-description");

  updateView(model_.get());

  if (!created_) {
    WLineEdit *password = resolve<WLineEdit *>
      (RegistrationModel::ChoosePasswordField);
    WLineEdit *password2 = resolve<WLineEdit *>
      (RegistrationModel::RepeatPasswordField);
    WText *password2Info = resolve<WText *>
      (RegistrationModel::RepeatPasswordField + std::string("-info"));

    if (password && password2 && password2Info)
      model_->validatePasswordsMatchJS(password, password2, password2Info);
    else
      bindEmpty("password-description");
  }

  WAnchor *isYou = resolve<WAnchor *>("confirm-is-you");
  if (!isYou) {
    std::unique_ptr<WAnchor> newIsYou
      (isYou = new WAnchor(std::string("#"), tr("Wt.Auth.confirm-is-you")));
    newIsYou->hide();
    bindWidget("confirm-is-you", std::move(newIsYou));
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

      WContainerWidget *icons = 
	bindWidget("icons", cpp14::make_unique<WContainerWidget>());
      icons->addStyleClass("Wt-field");

      for (unsigned i = 0; i < model_->oAuth().size(); ++i) {
	const OAuthService *service = model_->oAuth()[i];

	OAuthWidget *w
	  = icons->addWidget(cpp14::make_unique<OAuthWidget>(*service));
	w->authenticated().connect(this, &RegistrationWidget::oAuthDone);
      }
    }
  } else {
    setCondition("if:oauth", false);
    bindEmpty("icons");
  }

  if (!created_) {
    WPushButton *okButton =
      bindWidget("ok-button",
		 cpp14::make_unique<WPushButton>(tr("Wt.Auth.register")));
    WPushButton *cancelButton =
      bindWidget("cancel-button",
                 cpp14::make_unique<WPushButton>(tr("Wt.WMessageBox.Cancel")));

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
  updateModelField(model_.get(), RegistrationModel::LoginNameField);
  model_->validateField(RegistrationModel::LoginNameField);
  model_->setValidated(RegistrationModel::LoginNameField, false);
  update();
}

void RegistrationWidget::checkPassword()
{
  updateModelField(model_.get(), RegistrationModel::LoginNameField);
  updateModelField(model_.get(), RegistrationModel::ChoosePasswordField);
  updateModelField(model_.get(), RegistrationModel::EmailField);
  model_->validateField(RegistrationModel::ChoosePasswordField);
  model_->setValidated(RegistrationModel::ChoosePasswordField, false);
  update();
}

void RegistrationWidget::checkPassword2()
{
  updateModelField(model_.get(), RegistrationModel::ChoosePasswordField);
  updateModelField(model_.get(), RegistrationModel::RepeatPasswordField);
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
  std::unique_ptr<AbstractUserDatabase::Transaction>
    t(model_->users().startTransaction());

  updateModel(model_.get());

  if (validate()) {
    User user = model_->doRegister();
    if (user.isValid()) {
      registerUserDetails(user);
      if (!model_->baseAuth()->emailVerificationRequired() || user.unverifiedEmail().empty())
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
  removeFromParent();
}

void RegistrationWidget::confirmIsYou()
{
  updateModel(model_.get());

  switch (model_->confirmIsExistingUser()) {
  case IdentityConfirmationMethod::ConfirmWithPassword:
    {
      confirmPasswordLogin_.reset(new Login());
      confirmPasswordLogin_->login(model_->existingUser(), LoginState::Weak);
      confirmPasswordLogin_
	->changed().connect(this, &RegistrationWidget::confirmedIsYou);

      WDialog *dialog =
	authWidget_->createPasswordPromptDialog(*confirmPasswordLogin_);
      dialog->show();
    }

    break;
  case IdentityConfirmationMethod::ConfirmWithEmail:
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
  if (confirmPasswordLogin_->state() == LoginState::Strong)
    model_->existingUserConfirmed();
  else
    confirmPasswordLogin_.reset();
}

  }
}
