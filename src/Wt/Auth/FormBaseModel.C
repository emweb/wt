/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/Auth/AuthService.h"
#include "Wt/Auth/FormBaseModel.h"
#include "web/WebUtils.h"

namespace skeletons {
  extern const char *AuthStrings_xml1;
}

namespace Wt {
  namespace Auth {

const FormBaseModel::Field FormBaseModel::LoginNameField = "user-name";

FormBaseModel::FormBaseModel(const AuthService& baseAuth,
			     AbstractUserDatabase& users)
  : baseAuth_(baseAuth),
    users_(users),
    passwordAuth_(nullptr)
{ 
  WApplication *app = WApplication::instance();
  app->builtinLocalizedStrings().useBuiltin(skeletons::AuthStrings_xml1);
}

void FormBaseModel::addPasswordAuth(const AbstractPasswordService *auth)
{
  passwordAuth_ = auth;
}

void FormBaseModel::addOAuth(const OAuthService *auth)
{
  Utils::add(oAuth_, auth);
}

void FormBaseModel::addOAuth(const std::vector<const OAuthService *>& auth)
{
  for (unsigned i = 0; i < auth.size(); ++i)
    addOAuth(auth[i]);
}

WString FormBaseModel::label(Field field) const
{
  if (field == LoginNameField
      && baseAuth_.identityPolicy() == IdentityPolicy::EmailAddress)
    field = "email";

  return WString::tr(std::string("Wt.Auth.") + field);
}

void FormBaseModel::setValid(Field field)
{
  setValid(field, WString::Empty);
}

void FormBaseModel::setValid(Field field, const Wt::WString& message)
{
  setValidation(field,
		WValidator::Result(ValidationState::Valid,
				   message.empty() ? 
				   WString::tr("Wt.Auth.valid") : message));
}

bool FormBaseModel::loginUser(Login& login, User& user, LoginState state)
{
  if (!user.isValid())
    return false;

  if (user.status() == AccountStatus::Disabled) {
    setValidation
      (LoginNameField,
       WValidator::Result(ValidationState::Invalid,
			  WString::tr("Wt.Auth.account-disabled")));

    login.login(user, LoginState::Disabled);

    return false;
  } else if (baseAuth()->emailVerificationRequired() &&
	     user.email().empty()) {
    setValidation
      (LoginNameField,
       WValidator::Result(ValidationState::Invalid,
			  WString::tr("Wt.Auth.email-unverified")));

    login.login(user, LoginState::Disabled);

    return false;
  } else {
    login.login(user, state);

    return true;
  }  
}

  }
}


