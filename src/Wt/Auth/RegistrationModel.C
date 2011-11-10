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
    minUserNameLength_(4),
    passwordAuth_(0)
{
  if (baseAuth_.identityPolicy() == EmailAddressIdentity)
    validation_[UserName]
      = WValidator::Result(WValidator::Valid,
			   WString::tr("Wt.Auth.email-info"));
  else
    validation_[UserName]
      = WValidator::Result(WValidator::Valid,
			   WString::tr("Wt.Auth.user-name-info"));

  validation_[Password2]
    = WValidator::Result(WValidator::Valid,
			 WString::tr("Wt.Auth.password2-info"));

  validation_[Email]
    = WValidator::Result(WValidator::Valid,
			 WString::tr("Wt.Auth.email-info"));
}

void RegistrationModel::addPasswordAuth(const AbstractPasswordService *auth)
{
  passwordAuth_ = auth;

  validation_[Password]
    = WValidator::Result(WValidator::Valid,
			 auth->validatePassword(WString::Empty));
}

void RegistrationModel::addOAuth(const OAuthService *auth)
{
  oAuth_.push_back(auth);
}

void RegistrationModel::addOAuth(const std::vector<const OAuthService *>& auth)
{
  Utils::insert(oAuth_, auth);
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
      case UserNameIdentity:
	if (!idpIdentity_.name().empty())
	  values_[UserName] = idpIdentity_.name();
	else if (!idpIdentity_.email().empty()) {
	  std::string suggested = idpIdentity_.email();
	  std::size_t i = suggested.find('@');
	  if (i != std::string::npos)
	    suggested = suggested.substr(0, i);

	  values_[UserName] = WString::fromUTF8(suggested);
	}

	break;
      case EmailAddressIdentity:
	if (!idpIdentity_.email().empty())
	  values_[UserName] = WString::fromUTF8(idpIdentity_.email());
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
  case UserName:
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
      return baseAuth_.emailVerificationEnabled();
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
  case UserName:
    return baseAuth_.identityPolicy() == EmailAddressIdentity
      && idpIdentity_.isValid() && idpIdentity_.emailVerified();
  case Email:
    return idpIdentity_.isValid() && idpIdentity_.emailVerified();
  default:
    return false;
  }
}

WString RegistrationModel::validateUserName(const WT_USTRING& userName) const
{
  switch (baseAuth_.identityPolicy()) {
  case UserNameIdentity:
    if (static_cast<int>(userName.toUTF8().length()) < minUserNameLength_)
      return WString::tr("Wt.Auth.user-name-tooshort")
	.arg(minUserNameLength_);
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
  existingUser_ = users_.findWithIdentity("username", userName);
}

WString RegistrationModel::label(Field field) const
{
  switch (field) {
  case UserName:
    if (baseAuth_.identityPolicy() == EmailAddressIdentity)
      return WString::tr("Wt.Auth.email");
    else
      return WString::tr("Wt.Auth.user-name");
  case Password:
    return WString::tr("Wt.Auth.password");
  case Password2:
    return WString::tr("Wt.Auth.password2");
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
  case UserName:
    error = validateUserName(values_[field]);

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
    error = passwordAuth_->validatePassword(values_[field]);
    valid = error.empty();

    break;
  case Password2:
    if (values_[Password] != values_[Password2])
      error = WString::tr("Wt.Auth.passwords-dont-match");
    valid = error.empty();

    break;
  case Email:
    {
      std::string email = values_[Email].toUTF8();

      if (static_cast<int>(email.length()) < 3
	  || email.find('@') == std::string::npos)
	error = WString::tr("Wt.Auth.email-invalid");

      if (error.empty()) {
	User user = users_.findWithEmail(email);
	if (user.isValid())
	  error = WString::tr("Wt.Auth.email-exists");
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

  if (!validate(UserName))
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
     ""      WT_CLASS ".setHtml(i," + WString::tr("Wt.Auth.password2-info")
     .jsStringLiteral() + ");"
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
	user.addIdentity("username", values_[UserName]);

      std::string email;
      bool emailVerified = false;
      if (!idpIdentity_.email().empty()) {
	email = idpIdentity_.email();
	emailVerified = idpIdentity_.emailVerified();
      } else {
	if (baseAuth_.identityPolicy() == EmailAddressIdentity)
	  email = values_[UserName].toUTF8();
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
      user.addIdentity("username", values_[UserName]);

      passwordAuth_->updatePassword(user, values_[Password]);

      if (baseAuth_.emailVerificationEnabled()) {
	std::string email;
	if (baseAuth_.identityPolicy() == EmailAddressIdentity)
	  email = values_[UserName].toUTF8();
	else
	  email = values_[Email].toUTF8();

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
