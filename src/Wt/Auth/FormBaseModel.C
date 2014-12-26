/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/Auth/AuthService"
#include "Wt/Auth/FormBaseModel"
#include "web/WebUtils.h"

namespace skeletons {
  extern const char *AuthStrings_xml1;
}

namespace Wt {
  namespace Auth {

const FormBaseModel::Field FormBaseModel::LoginNameField = "user-name";

FormBaseModel::FormBaseModel(const AuthService& baseAuth,
			     AbstractUserDatabase& users,
			     WObject *parent)
  : WFormModel(parent), 
    baseAuth_(baseAuth),
    users_(users),
    passwordAuth_(0)
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
    addOAuth(auth);
}

WString FormBaseModel::label(Field field) const
{
  if (field == LoginNameField
      && baseAuth_.identityPolicy() == EmailAddressIdentity)
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
		WValidator::Result(WValidator::Valid,
				   message.empty() ? 
				   WString::tr("Wt.Auth.valid") : message));
}

bool FormBaseModel::loginUser(Login& login, User& user, LoginState state)
{
  if (!user.isValid())
    return false;

  if (user.status() == User::Disabled) {
    setValidation
      (LoginNameField,
       WValidator::Result(WValidator::Invalid,
			  WString::tr("Wt.Auth.account-disabled")));

    login.login(user, DisabledLogin);

    return false;
  } else if (baseAuth()->emailVerificationRequired() &&
	     user.email().empty()) {
    setValidation
      (LoginNameField,
       WValidator::Result(WValidator::Invalid,
			  WString::tr("Wt.Auth.email-unverified")));

    login.login(user, DisabledLogin);

    return false;
  } else {
    login.login(user, state);

    return true;
  }  
}

  }
}


