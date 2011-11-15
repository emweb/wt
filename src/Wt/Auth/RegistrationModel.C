/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService"
#include "Wt/Auth/AbstractUserDatabase"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/Login"
#include "Wt/Auth/RegistrationModel"

#include "Wt/WLineEdit"
#include "Wt/WText"

#include "web/Utils.h"

namespace Wt {
  namespace Auth {

RegistrationModel::RegistrationModel(const AuthService& baseAuth,
				     AbstractUserDatabase& users,
				     Login& login,
				     WObject *parent)
  : WObject(parent),
    baseAuth_(baseAuth),
    users_(users),
    login_(login),
    minLoginNameLength_(4),
    emailPolicy_(EmailDisabled),
    passwordAuth_(0)
{
  if (baseAuth_.identityPolicy() == EmailAddressIdentity)
    validation_[LoginName]
      = WValidator::Result(WValidator::Valid,
			   WString::tr("Wt.Auth.email-info"));
  else {
    if (baseAuth_.emailVerificationEnabled())
      emailPolicy_ = EmailOptional;
    else
      emailPolicy_ = EmailDisabled;

    validation_[LoginName]
      = WValidator::Result(WValidator::Valid,
			   WString::tr("Wt.Auth.user-name-info"));
  }

  validation_[Password]
    = WValidator::Result(WValidator::Valid,
			 WString::tr("Wt.Auth.choose-password-info"));;

  validation_[Password2]
    = WValidator::Result(WValidator::Valid,
			 WString::tr("Wt.Auth.repeat-password-info"));

  setEmailPolicy(emailPolicy_);
}

void RegistrationModel::addPasswordAuth(const AbstractPasswordService *auth)
{
  passwordAuth_ = auth;
}

void RegistrationModel::addOAuth(const OAuthService *auth)
{
  oAuth_.push_back(auth);
}

void RegistrationModel::addOAuth(const std::vector<const OAuthService *>& auth)
{
  Utils::insert(oAuth_, auth);
}

void RegistrationModel::setEmailPolicy(EmailPolicy policy)
{
  emailPolicy_ = policy;
  switch (emailPolicy_) {
  case EmailMandatory:
    validation_[Email]
      = WValidator::Result(WValidator::Valid,
			   WString::tr("Wt.Auth.email-info"));
    break;
  case EmailOptional:
    validation_[Email]
      = WValidator::Result(WValidator::Valid,
			   WString::tr("Wt.Auth.optional-email-info"));
    break;
  default:
    break;
  }
}

bool RegistrationModel::registerIdentified(const Identity& identity)
{
  idpIdentity_ = identity;

  if (idpIdentity_.isValid()) {
    User user = baseAuth_.identifyUser(idpIdentity_, users_);

    if (user.isValid()) {
      login_.login(user);
      return true;
    } else {
      switch (baseAuth_.identityPolicy()) {
      case LoginNameIdentity:
	if (!idpIdentity_.name().empty())
	  values_[LoginName] = idpIdentity_.name();
	else if (!idpIdentity_.email().empty()) {
	  std::string suggested = idpIdentity_.email();
	  std::size_t i = suggested.find('@');
	  if (i != std::string::npos)
	    suggested = suggested.substr(0, i);

	  values_[LoginName] = WString::fromUTF8(suggested);
	}

	break;
      case EmailAddressIdentity:
	if (!idpIdentity_.email().empty())
	  values_[LoginName] = WString::fromUTF8(idpIdentity_.email());
	break;

      default:
	break;
      }

      if (!idpIdentity_.email().empty())
	values_[Email] = idpIdentity_.email();

      return false;
    }
  } else
    return false;
}

bool RegistrationModel::isVisible(Field field) const
{
  switch (field) {
  case LoginName:
    if (baseAuth_.identityPolicy() == OptionalIdentity)
      return passwordAuth_ && !idpIdentity_.isValid();
    else
      return true;
  case Password:
  case Password2:
    return passwordAuth_ && !idpIdentity_.isValid();
  case Email:
    if (baseAuth_.identityPolicy() == EmailAddressIdentity)
      return false;
    else
      if (emailPolicy_ == EmailDisabled)
	return false;
      else
	return true;
  default:
    return false;
  }
}

bool RegistrationModel::isFederatedLoginVisible() const
{
  return !oAuth_.empty() && !idpIdentity_.isValid();
}

bool RegistrationModel::isConfirmUserButtonVisible() const
{
  return confirmIsExistingUser() != ConfirmationNotPossible;
}

bool RegistrationModel::isReadOnly(Field field) const
{
  switch (field) {
  case LoginName:
    return baseAuth_.identityPolicy() == EmailAddressIdentity
      && idpIdentity_.isValid() && idpIdentity_.emailVerified();
  case Email:
    return idpIdentity_.isValid() && idpIdentity_.emailVerified();
  default:
    return false;
  }
}

WString RegistrationModel::validateLoginName(const WT_USTRING& userName) const
{
  switch (baseAuth_.identityPolicy()) {
  case LoginNameIdentity:
    if (static_cast<int>(userName.toUTF8().length()) < minLoginNameLength_)
      return WString::tr("Wt.Auth.user-name-tooshort")
	.arg(minLoginNameLength_);
    else
      return WString::Empty;

  case EmailAddressIdentity:
    if (static_cast<int>(userName.toUTF8().length()) < 3
	|| userName.toUTF8().find('@') == std::string::npos)
      return WString::tr("Wt.Auth.email-invalid");
    else
      return WString::Empty;

  default:
    return WString::Empty;
  }

  return WString::Empty;
}

void RegistrationModel::checkUserExists(const WT_USTRING& userName)
{
  existingUser_ = users_.findWithIdentity(Identity::LoginName, userName);
}

WString RegistrationModel::label(Field field) const
{
  switch (field) {
  case LoginName:
    if (baseAuth_.identityPolicy() == EmailAddressIdentity)
      return WString::tr("Wt.Auth.email");
    else
      return WString::tr("Wt.Auth.user-name");
  case Password:
    return WString::tr("Wt.Auth.choose-password");
  case Password2:
    return WString::tr("Wt.Auth.repeat-password");
  case Email:
    return WString::tr("Wt.Auth.email");
  default:
    return WString::Empty;
  }
}

void RegistrationModel::setValue(Field field, const WString& value)
{
  values_[field] = value;
}

const WString& RegistrationModel::value(Field field) const
{
  return values_[field];
}

bool RegistrationModel::validate(Field field)
{
  if (!isVisible(field))
    return true;

  bool valid = true;
  WString error;

  switch (field) {
  case LoginName:
    error = validateLoginName(values_[field]);

    if (error.empty()) {
      checkUserExists(values_[field]);
      bool exists = existingUser_.isValid();
      valid = !exists;

      if (exists && confirmIsExistingUser() == ConfirmationNotPossible)
	error = WString::tr("Wt.Auth.user-name-exists");
    } else
      valid = false;

    break;
  case Password:
    {
      AbstractPasswordService::AbstractStrengthValidator *v
	= passwordAuth_->strengthValidator();
      if (v) {
	WValidator::Result r
	  = v->validate(values_[Password], values_[LoginName],
			values_[Email].toUTF8());
	valid = r.state() == WValidator::Valid;
	error = r.message();
      } else
	valid = true;
    }

    break;
  case Password2:
    if (values_[Password] != values_[Password2])
      error = WString::tr("Wt.Auth.passwords-dont-match");
    valid = error.empty();

    break;
  case Email:
    {
      std::string email = values_[Email].toUTF8();

      if (!email.empty()) {
	if (static_cast<int>(email.length()) < 3
	    || email.find('@') == std::string::npos)
	  error = WString::tr("Wt.Auth.email-invalid");

	if (error.empty()) {
	  User user = users_.findWithEmail(email);
	  if (user.isValid())
	    error = WString::tr("Wt.Auth.email-exists");
	}
      } else {
	if (emailPolicy_ != EmailOptional)
	  error = WString::tr("Wt.Auth.email-invalid");
      }

      valid = error.empty();

      break;
    }
  default:
    return true;
  }

  if (valid)
    validation_[field] = WValidator::Result(WValidator::Valid,
					    WString::tr("Wt.Auth.valid"));
  else
    validation_[field] = WValidator::Result(WValidator::Invalid, error);

  return validation_[field].state() == WValidator::Valid;
}

const WValidator::Result&
RegistrationModel::validationResult(Field field) const
{
  return validation_[field];
}

RegistrationModel::IdentityConfirmationMethod
RegistrationModel::confirmIsExistingUser() const
{
  if (existingUser_.isValid()) {
    if (!existingUser_.password().empty())
      return ConfirmWithPassword;
    else
      if (baseAuth_.emailVerificationEnabled()
	  && !existingUser_.email().empty())
	return ConfirmWithEmail;
  }

  return ConfirmationNotPossible;
}
  
void RegistrationModel::existingUserConfirmed()
{
  if (idpIdentity_.isValid())
    existingUser_.addIdentity(idpIdentity_.provider(),
			      WT_USTRING::fromUTF8(idpIdentity_.id()));

  login_.login(existingUser_);
}

bool RegistrationModel::validate()
{
  bool valid = true;

  if (!validate(LoginName))
    valid = false;

  if (!validate(Password))
    valid = false;

  if (!validate(Password2))
    valid = false;

  if (!validate(Email))
    valid = false;

  return valid;
}

bool RegistrationModel::valid() const
{
  for (unsigned i = 0; i < 4; ++i)
    if (validation_[i].state() != WValidator::Valid)
      return false;

  return true;
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
  if (!passwordAuth_ && !idpIdentity_.isValid()) {
    // FIXME: Set message that you need to identify using oauth.
    return User();
  } else {
    std::auto_ptr<AbstractUserDatabase::Transaction>
      t(users_.startTransaction());

    User user = users_.registerNew();

    if (idpIdentity_.isValid()) {
      user.addIdentity(idpIdentity_.provider(),
		       WT_USTRING::fromUTF8(idpIdentity_.id()));

      if (baseAuth_.identityPolicy() != OptionalIdentity)
	user.addIdentity(Identity::LoginName, values_[LoginName]);

      std::string email;
      bool emailVerified = false;
      if (!idpIdentity_.email().empty()) {
	email = idpIdentity_.email();
	emailVerified = idpIdentity_.emailVerified();
      } else {
	if (baseAuth_.identityPolicy() == EmailAddressIdentity)
	  email = values_[LoginName].toUTF8();
	else
	  email = values_[Email].toUTF8();
      }

      if (!email.empty()) {
	if (emailVerified || !baseAuth_.emailVerificationEnabled())
	  user.setEmail(email);
	else
	  baseAuth_.verifyEmailAddress(user, email);
      }
    } else {
      user.addIdentity(Identity::LoginName, values_[LoginName]);

      passwordAuth_->updatePassword(user, values_[Password]);

      if (baseAuth_.emailVerificationEnabled()) {
	std::string email;
	if (baseAuth_.identityPolicy() == EmailAddressIdentity)
	  email = values_[LoginName].toUTF8();
	else
	  email = values_[Email].toUTF8();

	if (!email.empty())
	  baseAuth_.verifyEmailAddress(user, email);
      }
    }

    if (t.get())
      t->commit();

    return user;
  }
}

  }
}
