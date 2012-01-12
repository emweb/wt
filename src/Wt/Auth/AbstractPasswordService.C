/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Auth/AbstractPasswordService"

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
    
  }
}
