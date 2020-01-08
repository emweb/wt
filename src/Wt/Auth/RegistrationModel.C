/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService.h"
#include "Wt/Auth/AbstractUserDatabase.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/Login.h"
#include "Wt/Auth/RegistrationModel.h"

#include "Wt/WLineEdit.h"
#include "Wt/WText.h"

#include <memory>

namespace Wt {
  namespace Auth {

const WFormModel::Field RegistrationModel::ChoosePasswordField
  = "choose-password";

const WFormModel::Field RegistrationModel::RepeatPasswordField
  = "repeat-password";

const WFormModel::Field RegistrationModel::EmailField
  = "email";

RegistrationModel::RegistrationModel(const AuthService& baseAuth,
				     AbstractUserDatabase& users,
				     Login& login)
  : FormBaseModel(baseAuth, users),
    login_(login),
    minLoginNameLength_(4),
    emailPolicy_(EmailPolicy::Disabled)
{
  if (baseAuth.identityPolicy() != IdentityPolicy::EmailAddress) {
    if (baseAuth.emailVerificationRequired())
      emailPolicy_ = EmailPolicy::Mandatory;
    else if (baseAuth.emailVerificationEnabled())
      emailPolicy_ = EmailPolicy::Optional;
    else
      emailPolicy_ = EmailPolicy::Disabled;
  }

  reset();
}

void RegistrationModel::reset()
{
  idpIdentity_ = Identity();
  existingUser_ = User();

  if (baseAuth()->identityPolicy() == IdentityPolicy::EmailAddress)
    addField(LoginNameField, WString::tr("Wt.Auth.email-info"));
  else
    addField(LoginNameField, WString::tr("Wt.Auth.user-name-info"));

  addField(ChoosePasswordField, WString::tr("Wt.Auth.choose-password-info"));
  addField(RepeatPasswordField, WString::tr("Wt.Auth.repeat-password-info"));

  setEmailPolicy(emailPolicy_);
}

void RegistrationModel::setEmailPolicy(EmailPolicy policy)
{
  emailPolicy_ = policy;
  switch (emailPolicy_) {
  case EmailPolicy::Mandatory:
    addField(EmailField, WString::tr("Wt.Auth.email-info"));
    break;
  case EmailPolicy::Optional:
    addField(EmailField, WString::tr("Wt.Auth.optional-email-info"));
    break;
  default:
    break;
  }
}

bool RegistrationModel::registerIdentified(const Identity& identity)
{
  idpIdentity_ = identity;

  if (idpIdentity_.isValid()) {
    User user = baseAuth()->identifyUser(idpIdentity_, users());

    if (user.isValid()) {
      return loginUser(login_, user);
    } else {
      switch (baseAuth()->identityPolicy()) {
      case IdentityPolicy::LoginName:
	if (!idpIdentity_.name().empty())
	  setValue(LoginNameField, idpIdentity_.name());
	else if (!idpIdentity_.email().empty()) {
	  std::string suggested = idpIdentity_.email();
	  std::size_t i = suggested.find('@');
	  if (i != std::string::npos)
	    suggested = suggested.substr(0, i);

	  setValue(LoginNameField, WString::fromUTF8(suggested));
	}

	break;
      case IdentityPolicy::EmailAddress:
	if (!idpIdentity_.email().empty())
	  setValue(LoginNameField, WString::fromUTF8(idpIdentity_.email()));
	break;

      default:
	break;
      }

      if (!idpIdentity_.email().empty()) {
	setValue(EmailField, idpIdentity_.email());
	setValidation(EmailField, 
		      WValidator::Result(ValidationState::Valid, 
					 WString::Empty));
      }

      return false;
    }
  } else
    return false;
}

bool RegistrationModel::isVisible(Field field) const
{
  if (field == LoginNameField) {
    if (baseAuth()->identityPolicy() == IdentityPolicy::Optional)
      return passwordAuth() && !idpIdentity_.isValid();
    else
      return true;
  } else if (field == ChoosePasswordField || field == RepeatPasswordField) {
    return passwordAuth() && !idpIdentity_.isValid();
  } else if (field == EmailField) {
    if (baseAuth()->identityPolicy() == IdentityPolicy::EmailAddress)
      return false;
    else
      if (emailPolicy_ == EmailPolicy::Disabled)
	return false;
      else
	return true;
  } else 
    return true;
}

bool RegistrationModel::isFederatedLoginVisible() const
{
  return !oAuth().empty() && !idpIdentity_.isValid();
}

bool RegistrationModel::isConfirmUserButtonVisible() const
{
  return confirmIsExistingUser() != 
    IdentityConfirmationMethod::ConfirmationNotPossible;
}

bool RegistrationModel::isReadOnly(Field field) const
{
  if (FormBaseModel::isReadOnly(field))
    return true;

  if (field == LoginNameField)
    return baseAuth()->identityPolicy() == IdentityPolicy::EmailAddress
      && idpIdentity_.isValid() && idpIdentity_.emailVerified();
  else if (field == EmailField)
    return idpIdentity_.isValid() && idpIdentity_.emailVerified();
  else
    return false;
}

WString RegistrationModel::validateLoginName(const WT_USTRING& userName) const
{
  switch (baseAuth()->identityPolicy()) {
  case IdentityPolicy::LoginName:
    if (static_cast<int>(userName.toUTF8().length()) < minLoginNameLength_)
      return WString::tr("Wt.Auth.user-name-tooshort")
	.arg(minLoginNameLength_);
    else
      return WString::Empty;

  case IdentityPolicy::EmailAddress:
    if (static_cast<int>(userName.toUTF8().length()) < 3
	|| userName.toUTF8().find('@') == std::string::npos)
      return WString::tr("Wt.Auth.email-invalid");
    else
      return WString::Empty;

  default:
    return WString::Empty;
  }
}

void RegistrationModel::checkUserExists(const WT_USTRING& userName)
{
  existingUser_ = users().findWithIdentity(Identity::LoginName, userName);
}

bool RegistrationModel::validateField(Field field)
{
  if (!isVisible(field))
    return true;

  bool valid = true;
  WString error;

  if (field == LoginNameField) {
    error = validateLoginName(valueText(field));

    if (error.empty()) {
      checkUserExists(valueText(field));
      bool exists = existingUser_.isValid();
      valid = !exists;

      if (exists && confirmIsExistingUser() == 
	  IdentityConfirmationMethod::ConfirmationNotPossible)
	error = WString::tr("Wt.Auth.user-name-exists");
    } else
      valid = false;

    if (isReadOnly(field))
      valid = true;
  } else if (field == ChoosePasswordField) {
    AbstractPasswordService::AbstractStrengthValidator *v
      = passwordAuth()->strengthValidator();

    if (v) {
      WValidator::Result r
	= v->validate(valueText(ChoosePasswordField),
		      valueText(LoginNameField),
		      valueText(EmailField).toUTF8());
      valid = r.state() == ValidationState::Valid;
      error = r.message();
    } else
      valid = true;
  } else if (field == RepeatPasswordField) {
    if (validation(ChoosePasswordField).state() == ValidationState::Valid) {
      if (valueText(ChoosePasswordField) != valueText(RepeatPasswordField))
	error = WString::tr("Wt.Auth.passwords-dont-match");
      valid = error.empty();
    } else
      return true; // Do not validate the repeat field yet
  } else if (field == EmailField) {
    std::string email = valueText(EmailField).toUTF8();

    if (!email.empty()) {
      if (static_cast<int>(email.length()) < 3
	  || email.find('@') == std::string::npos)
	error = WString::tr("Wt.Auth.email-invalid");

      if (error.empty()) {
	User user = users().findWithEmail(email);
	if (user.isValid())
	  error = WString::tr("Wt.Auth.email-exists");
      }
    } else {
      if (emailPolicy_ != EmailPolicy::Optional)
	error = WString::tr("Wt.Auth.email-invalid");
    }

    valid = error.empty();
  } else
    return true;

  if (valid)
    setValid(field, error);
  else
    setValidation(field, WValidator::Result(ValidationState::Invalid, error));

  return validation(field).state() == ValidationState::Valid;
}

IdentityConfirmationMethod
RegistrationModel::confirmIsExistingUser() const
{
  if (existingUser_.isValid()) {
    if (!existingUser_.password().empty())
      return IdentityConfirmationMethod::ConfirmWithPassword;
    else
      if (baseAuth()->emailVerificationEnabled()
	  && !existingUser_.email().empty())
	return IdentityConfirmationMethod::ConfirmWithEmail;
  }

  return IdentityConfirmationMethod::ConfirmationNotPossible;
}
  
void RegistrationModel::existingUserConfirmed()
{
  if (idpIdentity_.isValid())
    existingUser_.addIdentity(idpIdentity_.provider(),
			      WT_USTRING::fromUTF8(idpIdentity_.id()));

  loginUser(login_, existingUser_);
}

void RegistrationModel::validatePasswordsMatchJS(WLineEdit *password,
						 WLineEdit *password2,
						 WText *info2)
{
  // FIXME this should be a WValidator really and also does not really
  // belong here ?
  password2->keyWentUp().connect
    ("function(o) {"
     """var i=" + info2->jsRef() + ",o1=" + password->jsRef() + ";"
     """if (!$(o1).hasClass('Wt-invalid')) {"
     ""  "if (o.value == o1.value) {"
     ""     "$(o).removeClass('Wt-invalid');"
     ""      WT_CLASS ".setHtml(i," + WString::tr("Wt.Auth.valid")
     .jsStringLiteral() + ");"
     ""  "} else {"
     ""     "$(o).removeClass('Wt-valid');"
     ""      WT_CLASS ".setHtml(i,"
     ""        + WString::tr("Wt.Auth.repeat-password-info").jsStringLiteral()
     +                         ");"
     ""  "}"
     """}"
     "}");
}

User RegistrationModel::doRegister()
{
  if (!passwordAuth() && !idpIdentity_.isValid()) {
    // FIXME: Set message that you need to identify using oauth.
    return User();
  } else {
    User user = users().registerNew();

    if (idpIdentity_.isValid()) {
      user.addIdentity(idpIdentity_.provider(),
		       WT_USTRING::fromUTF8(idpIdentity_.id()));

      if (baseAuth()->identityPolicy() != IdentityPolicy::Optional)
	user.addIdentity(Identity::LoginName, valueText(LoginNameField));

      std::string email;
      bool emailVerified = false;
      if (!idpIdentity_.email().empty()) {
	email = idpIdentity_.email();
	emailVerified = idpIdentity_.emailVerified();
      } else {
	if (baseAuth()->identityPolicy() == IdentityPolicy::EmailAddress)
	  email = valueText(LoginNameField).toUTF8();
	else
	  email = valueText(EmailField).toUTF8();
      }

      if (!email.empty()) {
	if (emailVerified || !baseAuth()->emailVerificationEnabled())
	  user.setEmail(email);
	else
	  baseAuth()->verifyEmailAddress(user, email);
      }
    } else {
      user.addIdentity(Identity::LoginName, valueText(LoginNameField));

      passwordAuth()->updatePassword(user, valueText(ChoosePasswordField));

      if (baseAuth()->emailVerificationEnabled()) {
	std::string email;
	if (baseAuth()->identityPolicy() == IdentityPolicy::EmailAddress)
	  email = valueText(LoginNameField).toUTF8();
	else
	  email = valueText(EmailField).toUTF8();

	if (!email.empty())
	  baseAuth()->verifyEmailAddress(user, email);
      }
    }

    return user;
  }
}

  }
}
