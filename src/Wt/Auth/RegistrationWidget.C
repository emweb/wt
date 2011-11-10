/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthWidget"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/Login"
#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/RegistrationWidget"

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WDialog"
#include "Wt/WImage"
#include "Wt/WLineEdit"
#include "Wt/WPushButton"
// #include "Wt/WRegExpValidator"
#include "Wt/WText"

namespace skeletons {
  extern const char *Auth_xml1;
}

namespace Wt {
  namespace Auth {

RegistrationWidget::RegistrationWidget(const AuthService& baseAuth,
				       AbstractUserDatabase& users,
				       Login& login,
				       AuthWidget *authWidget)
  : WTemplate(tr("Wt.Auth.template.registration")),
    validated_(false),
    authWidget_(authWidget),
    created_(false),
    confirmPasswordLogin_(0)
{
  addFunction("id", &WTemplate::Functions::id);
  addFunction("tr", &WTemplate::Functions::tr);

  WApplication *app = WApplication::instance();
  app->builtinLocalizedStrings().useBuiltin(skeletons::Auth_xml1);

  model_ = new RegistrationModel(baseAuth, users, login, this);

  login.changed().connect(this, &RegistrationWidget::close);
}

void RegistrationWidget::render(WFlags<RenderFlag> flags)
{
  if (!created_) {
    update();
    created_ = true;
  }

  WTemplate::render(flags);
}

WFormWidget *RegistrationWidget::createField(RegistrationModel::Field field)
{
  WFormWidget *result;

  switch (field) {
  case RegistrationModel::UserName:
    result = new WLineEdit();
    result->changed().connect
      (boost::bind(&RegistrationWidget::checkUserName, this));

    break;
  case RegistrationModel::Email:
    result = new WLineEdit();

    break;
  case RegistrationModel::Password:
    {
      WLineEdit *p = new WLineEdit();
      p->setEchoMode(WLineEdit::Password);
      p->changed().connect
	(boost::bind(&RegistrationWidget::checkPassword, this));
      result = p;
    }

    break;
  case RegistrationModel::Password2:
    {
      WLineEdit *p = new WLineEdit();
      p->setEchoMode(WLineEdit::Password);
      p->changed().connect
	(boost::bind(&RegistrationWidget::checkPassword2, this));
      result = p;
    }

    break;
  default:
    result = 0;
  }

  return result;
}

void RegistrationWidget::updateField(const std::string& var,
				     RegistrationModel::Field field)
{
  if (model_->isVisible(field)) {
    setCondition("if:" + var, true);
    WFormWidget *edit = resolve<WFormWidget *>(var);
    if (!edit) {
      edit = createField(field);
      bindWidget(var, edit);
    }

    edit->setValueText(model_->value(field));

    WText *info = resolve<WText *>(var + "-info");
    if (!info) {
      info = new WText();
      bindWidget(var + "-info", info);
    }

    bindString(var + "-label", model_->label(field));

    const WValidator::Result& v = model_->validationResult(field);
    info->setText(v.message());

    switch (v.state()) {
    case WValidator::InvalidEmpty:
    case WValidator::Invalid:
      edit->removeStyleClass("Wt-valid");
      if (validated_)
	edit->addStyleClass("Wt-invalid");
      info->addStyleClass("Wt-error");

      break;
    case WValidator::Valid:
      edit->removeStyleClass("Wt-invalid");
      if (validated_)
	edit->addStyleClass("Wt-valid");
      info->removeStyleClass("Wt-error");
    }

    edit->setDisabled(model_->isReadOnly(field));
    edit->toggleStyleClass("Wt-disabled", edit->isDisabled());
  } else {
    setCondition("if:" + var, false);
    bindEmpty(var);
    bindEmpty(var + "-info");    
  }
}

void RegistrationWidget::update()
{
  if (model_->passwordAuth() && !model_->oAuth().empty())
    bindString("password-description",
	       tr("Wt.Auth.password-or-oauth-registration"));
  else
    bindEmpty("password-description");

  updateField("user-name", RegistrationModel::UserName);
  updateField("password", RegistrationModel::Password);
  updateField("password2", RegistrationModel::Password2);
  updateField("email", RegistrationModel::Email);

  if (!created_) {
    WLineEdit *password = resolve<WLineEdit *>("password");
    WLineEdit *password2 = resolve<WLineEdit *>("password2");
    WText *password2Info = resolve<WText *>("password2-info");

    if (password && password2 && password2Info)
      model_->validatePasswordsMatchJS(password, password2, password2Info);
  }

  WAnchor *isYou = resolve<WAnchor *>("confirm-is-you");
  if (!isYou) {
    isYou = new WAnchor("#", tr("Wt.Auth.confirm-is-you"));
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
	OAuthProcess *process
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
    Wt::log("auth")
      << oauth->service().name() << ": identified: as "
      << identity.id() << ", " << identity.name() << ", " << identity.email();

    if (!model_->registerIdentified(identity))
      update();
  } else {
    if (authWidget_)
      authWidget_->displayError(oauth->error());
    Wt::log("auth") << oauth->service().name() << ": error: " << oauth->error();
  }
}

void RegistrationWidget::updateModel(const std::string& var,
				     RegistrationModel::Field field)
{
  WFormWidget *edit = resolve<WFormWidget *>(var);
  if (edit)
    model_->setValue(field, edit->valueText());
}

void RegistrationWidget::updateModel()
{
  updateModel("user-name", RegistrationModel::UserName);
  updateModel("password", RegistrationModel::Password);
  updateModel("password2", RegistrationModel::Password2);
  updateModel("email", RegistrationModel::Email);
}

void RegistrationWidget::checkUserName()
{
  updateModel("user-name", RegistrationModel::UserName);
  model_->validate(RegistrationModel::UserName);
  update();
}

void RegistrationWidget::checkPassword()
{
  updateModel("password", RegistrationModel::Password);
  model_->validate(RegistrationModel::Password);
  update();
}

void RegistrationWidget::checkPassword2()
{
  updateModel("password", RegistrationModel::Password);
  updateModel("password2", RegistrationModel::Password2);
  model_->validate(RegistrationModel::Password2);
  update();
}

bool RegistrationWidget::validate()
{
  bool result = model_->validate();
  validated_ = true;
  return result;
}

void RegistrationWidget::doRegister()
{
  std::auto_ptr<AbstractUserDatabase::Transaction>
    t(model_->users().startTransaction());

  updateModel();

  if (validate()) {
    User user = model_->doRegister();
    if (user.isValid()) {
      registerUserDetails(user);
      model_->login().login(user);
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
  updateModel();

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

    Wt::log("notice") << "Confirming a new identity to existing user not "
      "yet implemented";

    break;
  default:
    Wt::log("error") << "That's gone haywire.";
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
