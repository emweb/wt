/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PasswordStrengthValidator"
#include "Wt/WString"

extern "C" {
  #include "passwdqc.h"
}

namespace {
  const char *reasons[] =
    { "ok",
      "error",
      "same",
      "similar",
      "short",
      "long",
      "simpleshort",
      "simple",
      "personal",
      "word",
      "seq"
    };
}

namespace Wt {
  namespace Auth {

const int PasswordStrengthValidator::Disabled = std::numeric_limits<int>::max();

PasswordStrengthValidator::PasswordStrengthValidator()
{ 
  minLength_[0] = Disabled;
  minLength_[1] = 15;
  minLength_[2] = 11;
  minLength_[3] = 8;
  minLength_[4] = 7;

  passPhraseWords_ = 3;
  minMatchLength_ = 4;
}

int PasswordStrengthValidator::evaluateStrength(const WT_USTRING& password,
						const WT_USTRING& loginName,
						const std::string& email) const
{
  passwdqc_params_qc_t params;
  for (unsigned i = 0; i < 5; ++i)
    params.min[i] = minLength_[i];
  params.passphrase_words = passPhraseWords_;
  params.match_length = minMatchLength_;
  params.similar_deny = false;
  params.random_bits = 0;
  params.max = 256;

  passwdqc_user_t user;
  user.pw_name = loginName.toUTF8().c_str();
  user.pw_email = email.c_str();
  
  return passwdqc_check(&params, password.toUTF8().c_str(), 0, &user);
}

bool PasswordStrengthValidator::isValid(int result) const
{
  return result == 0;
}

WString PasswordStrengthValidator::message(int result) const
{
  return WString::tr(std::string("Wt.Auth.passwdqc.reason-") + reasons[result]);
}

  }
}
