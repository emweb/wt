/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <string>

#include "Wt/WLogger.h"
#include "AuthUtils.h"
#include "HashFunction.h"
#include "PasswordHash.h"
#include "PasswordVerifier.h"

namespace Wt {

LOGGER("Auth.PasswordVerifier");

  namespace Auth {

PasswordVerifier::PasswordVerifier()
  : saltLength_(12)
{ }

PasswordVerifier::~PasswordVerifier()
{ }

void PasswordVerifier::addHashFunction(std::unique_ptr<HashFunction> function)
{
  hashFunctions_.push_back(std::move(function));
}

const std::vector<HashFunction *> PasswordVerifier::hashFunctions() const
{
  std::vector<HashFunction *> result;
  for (auto &hashFunction : hashFunctions_)
    result.push_back(hashFunction.get());
  return result;
}

bool PasswordVerifier::needsUpdate(const PasswordHash& hash) const
{
  return hash.function() != hashFunctions_[0]->name();
}

PasswordHash PasswordVerifier::hashPassword(const WString& password) const
{
  std::string msg = password.toUTF8();
  std::string salt = Utils::createSalt(saltLength_);
  salt = Utils::encodeAscii(salt);

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
      return f.verify(password.toUTF8(), hash.salt(), hash.value());
  }

  LOG_ERROR("verify() no hash configured for " << hash.function());

  return false;
}

  }
}
