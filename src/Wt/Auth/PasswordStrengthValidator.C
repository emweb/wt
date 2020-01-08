/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PasswordStrengthValidator.h"
#include "Wt/WString.h"

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

AbstractPasswordService::StrengthValidatorResult 
PasswordStrengthValidator::evaluateStrength(const WT_USTRING& password,
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

  std::string login_utf8 = loginName.toUTF8();
  passwdqc_user_t user;
  user.pw_name = login_utf8.c_str();
  user.pw_email = email.c_str();
  
  int index = passwdqc_check(&params, password.toUTF8().c_str(), nullptr, &user);

  WString message 
    = WString::tr(std::string("Wt.Auth.passwdqc.reason-") + reasons[index]);
  bool valid = index == 0;
  AbstractPasswordService::StrengthValidatorResult result(valid, 
							  message, 
							  valid ? 5 : 0);
  return result;
}

void PasswordStrengthValidator::setMinimumLength(PasswordStrengthType type,
						 int length)
{
  minLength_[static_cast<int>(type)] = length;
}

void PasswordStrengthValidator::setMinimumPassPhraseWords(int words)
{
  passPhraseWords_ = words;
}

void PasswordStrengthValidator::setMinimumMatchLength(int length)
{
  minMatchLength_ = length;
}

  }
}
