/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <string.h>

#include "Wt/WLogger"
#include "Wt/WRandom"
#include "HashFunction"
#include "PasswordHash"
#include "PasswordVerifier"
#include "Utils.h"

namespace Wt {
  namespace Auth {

PasswordVerifier::PasswordVerifier()
  : saltLength_(3)
{ }

PasswordVerifier::~PasswordVerifier()
{ 
  for (unsigned i = 0; i < hashFunctions_.size(); ++i)
    delete hashFunctions_[i];
}

void PasswordVerifier::addHashFunction(HashFunction *function)
{
  hashFunctions_.push_back(function);
}

bool PasswordVerifier::needsUpdate(const PasswordHash& hash) const
{
  return hash.function() != hashFunctions()[0]->name();
}

PasswordHash PasswordVerifier::hashPassword(const WString& password) const
{
  unsigned char *saltBuf = new unsigned char[saltLength_];
  for (int i = 0; i < saltLength_; i += 3) {
    unsigned r = WRandom::get();
    memcpy(saltBuf + i, &r, 3);
  }

  std::string msg = password.toUTF8();
  std::string salt
    = Utils::encodeAscii(std::string(saltBuf, saltBuf + saltLength_));

  const HashFunction& f = *hashFunctions_[0];
  std::string hash = f.compute(msg, salt);
  return PasswordHash(f.name(), salt, hash);
}

bool PasswordVerifier::verify(const WString& password,
			      const PasswordHash& hash) const
{
  for (unsigned i = 0; i < hashFunctions_.size(); ++i) {
    const HashFunction& f = *hashFunctions_[i];

    if (f.name() == hash.function())
      return f.compute(password.toUTF8(), hash.salt()) == hash.value();
  }

  Wt::log("error") << "PasswordVerifier::verify() no hash configured for "
		   << hash.function();

  return false;
}

  }
}
