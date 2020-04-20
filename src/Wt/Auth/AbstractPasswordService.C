/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService.h"

namespace Wt {
  namespace Auth {

AbstractPasswordService::~AbstractPasswordService()
{
}

AbstractPasswordService::StrengthValidatorResult
::StrengthValidatorResult(
			  bool valid, 
			  const WString &message,
			  int strength) 
  : valid_(valid), message_(message), strength_(strength)
{}

AbstractPasswordService::AbstractStrengthValidator::AbstractStrengthValidator()
{
  setMandatory(true);
}

WValidator::Result AbstractPasswordService::AbstractStrengthValidator
::validate(const WT_USTRING& password, const WT_USTRING& loginName,
	   const std::string& email) const
{
  if (!isMandatory() && password.empty()) {
      return Result(ValidationState::Valid);
  }

  AbstractPasswordService::StrengthValidatorResult result 
    = evaluateStrength(password, loginName, email);

  if (result.isValid()) {
    return WValidator::Result(ValidationState::Valid, result.message());
  } else if (isMandatory() && password.empty()) {
    return WValidator::Result(ValidationState::InvalidEmpty, result.message());
  } else {
    return WValidator::Result(ValidationState::Invalid, result.message());
  }
}

WValidator::Result AbstractPasswordService::AbstractStrengthValidator
::validate(const WT_USTRING& password) const
{
#ifndef WT_TARGET_JAVA
  return validate(password, WT_USTRING::Empty, "");
#else
  return validate(password, "", "");
#endif
}

  }
}
