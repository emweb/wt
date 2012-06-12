/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AuthService"
#include "Wt/Auth/FormBaseModel"
#include "web/WebUtils.h"

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
{ }

void FormBaseModel::addPasswordAuth(const AbstractPasswordService *auth)
{
  passwordAuth_ = auth;
}

void FormBaseModel::addOAuth(const OAuthService *auth)
{
  oAuth_.push_back(auth);
}

void FormBaseModel::addOAuth(const std::vector<const OAuthService *>& auth)
{
  Utils::insert(oAuth_, auth);
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
  setValidation(field,
		WValidator::Result(WValidator::Valid,
				   WString::tr("Wt.Auth.valid")));
}

  }
}


