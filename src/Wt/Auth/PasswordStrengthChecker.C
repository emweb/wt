/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PasswordStrengthChecker"
#include "Wt/WString"

namespace Wt {
  namespace Auth {

PasswordStrengthChecker::PasswordStrengthChecker()
{ }

PasswordStrengthChecker::~PasswordStrengthChecker()
{ }

int PasswordStrengthChecker::check(const WString& password) const
{
  return 1;
}

bool PasswordStrengthChecker::isValid(int result) const
{
  return result == 1;
}

WString PasswordStrengthChecker::invalidReason(int result) const
{
  return WString();
}

int PasswordStrengthChecker::strength(int result) const
{
  return result ? 0 : 5;
}

  }
}
