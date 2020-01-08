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

WValidator::Result AbstractPasswordService::AbstractStrengthValidator
::validate(const WT_USTRING& password, const WT_USTRING& loginName,
	   const std::string& email) const
{
  AbstractPasswordService::StrengthValidatorResult result 
    = evaluateStrength(password, loginName, email);

  return WValidator::Result
    (result.isValid() ? ValidationState::Valid : ValidationState::Invalid, 
     result.message());
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
